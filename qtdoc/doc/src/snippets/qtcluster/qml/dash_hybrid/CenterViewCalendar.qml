/****************************************************************************
**
** Copyright (C) 2016 Pelagicore AG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Neptune IVI UI.
**
** $QT_BEGIN_LICENSE:GPL-QTAS$
** Commercial License Usage
** Licensees holding valid commercial Qt Automotive Suite licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and The Qt Company.  For
** licensing terms and conditions see https://www.qt.io/terms-conditions.
** For further information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
** SPDX-License-Identifier: GPL-3.0
**
****************************************************************************/

import QtQuick 2.6

Item {
    id: calendarContainer

    property string appointment: "No appointments"
    property var currentDate
    property string date
    property string time
    opacity: 0.5
    property alias xTarget: startupAnimation.to
    property int defaultXPos: 200

    Image {
        id: image
        source: "image://etc/calendar.png"
    }

    Text {
        id: dateText
        anchors.top: image.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: image.horizontalCenter
        text: date
        color: "gray"
        font.pixelSize: 16
    }

    Text {
        id: timeText
        anchors.top: dateText.bottom
        anchors.horizontalCenter: image.horizontalCenter
        text: time
        color: "gray"
        font.pixelSize: 20
    }

    Text {
        anchors.top: timeText.bottom
        anchors.horizontalCenter: image.horizontalCenter
        text: appointment
        color: "lightGray"
        font.pixelSize: 14
    }

    Timer {
        id: fadeOutTimer
        interval: 5000
        running: false
        repeat: false
        onTriggered: {
            calendarContainer.opacity = 0.5
        }
    }

    Behavior on opacity { PropertyAnimation { duration: 500 } }

    PropertyAnimation on x {
        id: startupAnimation
        duration: 500
        easing.type: Easing.InCubic
        onStopped: {
            calendarContainer.opacity = 1.0
            fadeOutTimer.start()
        }
    }

    onVisibleChanged: {
        if (visible) {
            currentDate = new Date()
            date = currentDate.toLocaleDateString(Qt.locale("en_GB"))
            time = currentDate.toLocaleTimeString(Qt.locale("en_GB"), "hh:mm")
            x = defaultXPos
            startupAnimation.start()
        }
    }
}

