//
//  ChatMessageArea.h
//  interface/src/ui
//
//  Created by Ryan Huffman on 4/11/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_ChatMessageArea_h
#define hifi_ChatMessageArea_h

#include <QTextBrowser>

const int CHAT_MESSAGE_LINE_HEIGHT = 130;

class ChatMessageArea : public QTextBrowser {
    Q_OBJECT
public:
    ChatMessageArea();
    virtual void setHtml(const QString& html);

public slots:
    void updateLayout();

protected:
    virtual void wheelEvent(QWheelEvent* event);

};

#endif // hifi_ChatMessageArea_h
