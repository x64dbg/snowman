#ifndef SNOWMANVIEW_H
#define SNOWMANVIEW_H

#include <QWidget>
#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <bridgemain.h>

struct SnowmanRange
{
    duint start;
    duint end;
};

class SnowmanView : public QWidget
{
    Q_OBJECT
public:
    explicit SnowmanView(QWidget* parent = nullptr);
    void decompileAt(const SnowmanRange* ranges, duint count) const;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void populateInstructionsContextMenu(QMenu* menu) const;
    void populateCxxContextMenu(QMenu* menu) const;
    void jumpFromInstructionsView() const;
    void jumpFromCxxView() const;

private:
    QMainWindow* mSnowmanMainWindow;
    QAction* mJumpFromInstructionsViewAction;
    QAction* mJumpFromCxxViewAction;
};

#endif // SNOWMANVIEW_H
