#include <cassert>
#include <functional>
#include <memory>

#include <QAction>
#include <QByteArray>
#include <QDir>
#include <QFileDialog>
#include <QIcon>
#include <QList>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPushButton>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>

#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardAction>

#include "deduplicator.h"
#include "image.h"
#include "mainwindow.h"
#include "merger.h"
#include "processor.h"
#include "queueitem.h"
#include "settings.h"
#include "ui_mainwindow.h"

namespace myriad {
    
    namespace {
        
        /**
         * Generates a sequence of glob patterns that represent union of all MIME types named in a
         * specified list. The resulting pattern can be used in a name filter for a @c QFileDialog.
         * Based upon Qt's <tt>nameFilterForMime()</tt> fuction.
         * @param mimeNameList The list of supported MIME type names.
         * @return A glob pattern for the union of MIME types specified in @p mimeNameList.
         */
        
        QString globPatternsForMimeTypes(const QList<QByteArray> mimeNameList) {
            
            QMimeDatabase mimeDb;
            QString allGlobPatterns;
            
            for (const auto& mimeName : mimeNameList) {
            
                QMimeType mimeType{mimeDb.mimeTypeForName(mimeName)};
                if (mimeType.isValid()) {
                    
                    if (mimeType.isDefault()) {
                        allGlobPatterns = "*";
                        break;
                    }
                    else {
                        
                        const auto globPatterns = mimeType.globPatterns().join(QLatin1Char(' '));
                        if (!globPatterns.isEmpty()) {
                            
                            if (!allGlobPatterns.isEmpty()) {
                                allGlobPatterns += ' ';
                            }
                            allGlobPatterns += globPatterns;
                        }
                    }
                }
            }
            
            return allGlobPatterns;
        }
    
        /**
        * Sets up a @c QFileDialog to prepare it for prompting for a one or more input image files.
        * These files are filtered by the MIME types supported by Myriad. The @c QApplication
        * instance must be created before this function is called.
        * @param dialog The dialog object to be modified.
        */

        void configureFileInputDialog(QFileDialog * dialog) {

            dialog->setFileMode(QFileDialog::ExistingFiles);
            
            const auto supportedPatterns = globPatternsForMimeTypes(processing::supportedMimeTypes());
            if (!supportedPatterns.isEmpty()) {
                dialog->setNameFilter("All supported images(" + supportedPatterns + ")");
            }
        }
        
        /**
        * Sets up a @c QFileDialog to prepare it for prompting for a single input folder.
        * @param dialog The dialog object to be modified.
        */

        void configureFolderInputDialog(QFileDialog * dialog) {
            dialog->setFileMode(QFileDialog::Directory);
            dialog->setOption(QFileDialog::ShowDirsOnly, true);
        }
    }

    class MainWindow::Private {
        
        public:

            /**
            * Initialises the main window's private data upon construction.
            * @param q The owner instance of the public class.
            */

            Private(MainWindow * const q)
                : q{q} {
            }
            
            /**
             * Sets up the KDE actions that are available through interaction with the Myriad main
             * window, and binds them to their associated methods.
             */
            
            void initActions() {
                
                auto * const actions = q->actionCollection();
                
                auto * const addFilesAction     = new QAction{QIcon::fromTheme(QStringLiteral("document-new")), i18n("&Add Files"), q};
                auto * const addFolderAction    = new QAction{QIcon::fromTheme(QStringLiteral("folder-new")), i18n("Add Fo&lder"), q};
                auto * const clearTargetsAction = new QAction{QIcon::fromTheme(QStringLiteral("edit-clear-list")), i18n("&Clear All Targets"), q};
                auto * const processAction      = new QAction{QIcon::fromTheme(QStringLiteral("go-next")), i18n("Start &Processing"), q};
                
                actions->setDefaultShortcut(addFilesAction,  Qt::CTRL + Qt::Key_O);
                actions->setDefaultShortcut(addFolderAction, Qt::CTRL + Qt::SHIFT + Qt::Key_O);
                actions->setDefaultShortcut(processAction,   Qt::CTRL + Qt::Key_Enter);
                
                actions->addAction("add-files",  addFilesAction);
                actions->addAction("add-folder", addFolderAction);
                actions->addAction("clear",      clearTargetsAction);
                actions->addAction("process",    processAction);
                
                connect(addFilesAction,     &QAction::triggered, [this] {addTargets(promptForInputs(configureFileInputDialog));});
                connect(addFolderAction,    &QAction::triggered, [this] {addTargets(promptForInputs(configureFolderInputDialog));});
                connect(clearTargetsAction, &QAction::triggered, [this] {clearAllTargets();});
                connect(processAction,      &QAction::triggered, [this] {m_processor->start(q);});
                
                // AFACT the new Qt signal/slot syntax (using member function pointers and/or
                // lambdas) is not available for the KStandardAction binding functions. So we use
                // the traditional SLOT() syntax.
                
                KStandardAction::quit(q, SLOT(close()), actions);
            }
            
            /**
             * Initialises and tweaks the elements of the main window's UI. We can here fine-tune
             * aspects of the various widgets contained therein to an extent not possible using 
             * only Qt Designer.
             */
            
            void initUi() {
                
                m_ui->setupUi(q);
                m_ui->inputsListView->setModel(&m_queueModel);
                
                m_lastModeRadioButton = m_ui->mergeModeRadioButton;

                connect(m_ui->deduplicateModeRadioButton, &QRadioButton::toggled, [this] {
                    resetProcessorIfChecked<processing::Deduplicator>(m_ui->deduplicateModeRadioButton);
                });
                
                connect(m_ui->mergeModeRadioButton, &QRadioButton::toggled, [this] {
                    resetProcessorIfChecked<processing::Merger>(m_ui->mergeModeRadioButton);
                });
            }
            
            /**
             * @see MainWindow::inputs()
             */
            
            QStringList inputs() const {
                
                QStringList result;
                for (auto i = 0; i < m_queueModel.rowCount(); ++i) {
                    result << m_queueModel.item(i)->data(modelview::PathRole).toString();
                }
                return result;
            }
            
            /**
             * Restores state information about the main window from the Myriad configuration file,
             * where it should have been saved when the application was last closed. This must be
             * called after the GUI has been created and events have been bound to it, since some
             * state (like the processing mode) is restored by activating the relevant button and
             * letting the normal slots kick in.
             */
            
            void restoreState() {
                
                auto * const settings = Settings::self();
                addTargets(settings->inputs());
                
                const auto savedLastInputDir = settings->lastInputDir();
                if (!savedLastInputDir.isEmpty()) {
                    m_lastInputDir = settings->lastInputDir();
                }
                
                // The UI is set up to select the "merge" processing mode (and initialise the 
                // corresponding processor object) by default. So, we need not do anything if this
                // is also the mode saved from last time.
                
                const auto savedProcessingMode = settings->processingMode();
                if (savedProcessingMode == Settings::EnumProcessingMode::Deduplicate) {
                    m_ui->deduplicateModeRadioButton->setChecked(true);
                }
            }
            
            /**
             * Saves state information about the main window to the Myriad configuration file, so 
             * that it can be restored the next time the application is run.
             */
            
            void saveState() const {
                
                auto * const settings = Settings::self();
                settings->setInputs(inputs());
                settings->setLastInputDir(m_lastInputDir);
                
                m_processor->saveState(settings);
                settings->save();
            }
            
        private:
            
            /**
             * Adds specified files or folders to the input target list. These will be scanned for
             * duplicates when the main Myriad processing is performed.
             * @param targetPaths The list of filesystems paths identifying the targets to add.
             */
            
            void addTargets(const QStringList& targetPaths) {
                
                for (const auto& targetPath : targetPaths) {
                    if (!targetPath.isEmpty()) {
                        m_queueModel.appendRow(new modelview::QueueItem(targetPath));
                    }
                }
            }
            
            /**
             * Removes all targets from the input list.
             */
            
            void clearAllTargets() {
                m_queueModel.clear();
            }
            
            /**
             * Displays a dialog box with which the user can specify a collection of input files or
             * directories that should be added to the processing queue. The directory last chosen
             * is saved as the initial selection for the next time this action is performed.
             * @param configureDialog A function that can be called upon a @c QFileDialog to
             * configure it to prompt for whatever sort of input is appropriate for the current 
             * processing mode.
             * @return The list of targets that were selected from the dialog box.
             */
            
            QStringList promptForInputs(const std::function<void(QFileDialog *)> configureDialog) {

                auto * const dialog = new QFileDialog{q, i18n("Add Target"), m_lastInputDir};
                configureDialog(dialog);

                QStringList targetPaths;
                if (dialog->exec()) {
                    
                    targetPaths = dialog->selectedFiles();
                    if (!targetPaths.empty()) {
                        m_lastInputDir = dialog->directory().path();
                    }
                }
                
                return targetPaths;
            }
            
            /**
             * Gets status text indicating the current processing phase and the number of targets 
             * that this processing is acting upon.
             */
            
            QString statusMessage() {
                
                if (m_phase == processing::Phase::IDLE) {
                    return i18n("Ready.");
                }
                else {
                    
                    const QString action = [this] {
                        switch (m_phase) {
                            
                            case processing::Phase::SCANNING:
                                return i18n("Scanning");
                                
                            case processing::Phase::FINGERPRINTING:
                                return i18n("Fingerprinting");
                                
                            case processing::Phase::COMPARING:
                                return i18n("Comparing");
                                
                            default:
                                return i18n("Processing");
                        }
                    } ();
                    
                    return i18n("%1 %L2 files in %L3 folders")
                               .arg(action)
                               .arg(m_inputFileCount)
                               .arg(m_inputFolderCount);
                }
            }
            
            /**
             * Checks if the specified radio button is checked, and if it is, reinitialises the 
             * main processor object to perform a specified action, copying over settings from the
             * existing processor state as appropriate.
             * @tparam ProcessorType The type of processing to set up. This should be a child class
             * of Processor.
             * @param button The radio button associated with the new type of processing. No action
             * is taken if this is not checked.
             */
            
            template<typename ProcessorType>
            void resetProcessorIfChecked(QRadioButton * const button) {

                if (button->isChecked() && button != m_lastModeRadioButton) {

                    m_processor = std::make_unique<ProcessorType>(*m_processor);
                    m_lastModeRadioButton = button;
                }
            }
        
            MainWindow * const q;
            
            int m_inputFileCount   = 0;
            int m_inputFolderCount = 0;
            
            QString m_lastInputDir{QDir::homePath()};
            QRadioButton * m_lastModeRadioButton = nullptr;
            processing::Phase m_phase = processing::Phase::IDLE;
            std::unique_ptr<processing::Processor> m_processor = std::make_unique<processing::Merger>();
            QStandardItemModel m_queueModel{0, 1};
            Ui::MainWindow * const m_ui = new Ui::MainWindow;
    };
    
    MainWindow::MainWindow(const QString& caption, QWidget * const parent)
        : KXmlGuiWindow{parent}, d{std::make_unique<Private>(this)} {

        setCaption(caption);
        
        d->initUi();
        d->initActions();
        setupGUI(Default, QStringLiteral("myriadui.rc"));
        
        d->restoreState();
    }
    
    MainWindow::~MainWindow() {
    }
    
    QStringList MainWindow::inputs() const {
        return d->inputs();
    }
    
    bool MainWindow::queryClose() {
        d->saveState();
        return true;
    }
}

#include "moc_mainwindow.cpp"
