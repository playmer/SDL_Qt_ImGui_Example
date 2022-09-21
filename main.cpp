#include "QApplication"
#include "QMainWindow"
#include "QTextEdit"
#include "QWindow"

#include "DockManager.h"

class DockOwningMainWindow : public QMainWindow
{
public:
    
    explicit DockOwningMainWindow (QWidget *parent = nullptr) :
        QMainWindow(parent)
    {
        mDockManager = new ads::CDockManager(this);
    }

    ~DockOwningMainWindow ()
    {
    }

    ads::CDockManager* GetDockManager()
    {
        return mDockManager;
    }

private:
    
    // The main container for docking
    ads::CDockManager* mDockManager;
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Make a window. This type has support for docking and gives a 
    // central window in the middle of the docking panels that doesn't move.
    auto mainWindow = new DockOwningMainWindow;

    // Sets the default window size.
    mainWindow ->resize(850, 700);

    // Just here as an example
    auto textEdit = new QTextEdit;
    mainWindow ->GetDockManager()->addDockWidget(ads::CenterDockWidgetArea, new ads::CDockWidget("Text Edit", textEdit));

    {
        auto dockWidget = new ads::CDockWidget("SdlWindow2", mainWindow);
        dockWidget->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromContent);
        auto sdlWindow = new QWindow();
        auto sdlWidget = QWidget::createWindowContainer(sdlWindow, dockWidget);
        (void)sdlWindow->winId();


        dockWidget->setWidget(sdlWidget, ads::CDockWidget::ForceNoScrollArea);

        mainWindow->GetDockManager()->addDockWidget(ads::CenterDockWidgetArea, dockWidget);
        sdlWidget->setWindowTitle("SdlWindow2");
        sdlWidget->setMinimumSize(480, 320);
        sdlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        sdlWindow->show();
    }

    mainWindow->show();

    return QApplication::exec();
}