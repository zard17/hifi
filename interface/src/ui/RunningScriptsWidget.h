//
//  RunningScriptsWidget.h
//  interface/src/ui
//
//  Created by Mohammed Nafees on 03/28/2014.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_RunningScriptsWidget_h
#define hifi_RunningScriptsWidget_h

// Qt
#include <QDockWidget>

namespace Ui {
    class RunningScriptsWidget;
}

class RunningScriptsWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit RunningScriptsWidget(QDockWidget *parent = 0);
    ~RunningScriptsWidget();

    void setRunningScripts(const QStringList& list);

signals:
    void stopScriptName(const QString& name);

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void stopScript(int row, int column);
    void loadScript(int row, int column);
    void allScriptsStopped();

private:
    Ui::RunningScriptsWidget *ui;
    QStringList _recentlyLoadedScripts;
    QString _lastStoppedScript;

    void createRecentlyLoadedScriptsTable();
};

#endif // hifi_RunningScriptsWidget_h
