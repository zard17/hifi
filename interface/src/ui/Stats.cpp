//
//  Stats.cpp
//  interface/src/ui
//
//  Created by Lucas Crisman on 22/03/14.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <sstream>

#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Stats.h"
#include "InterfaceConfig.h"
#include "Menu.h"
#include "Util.h"

using namespace std;

const int STATS_PELS_PER_LINE = 20;

const int STATS_GENERAL_MIN_WIDTH = 165; 
const int STATS_PING_MIN_WIDTH = 190;
const int STATS_GEO_MIN_WIDTH = 240;
const int STATS_VOXEL_MIN_WIDTH = 410;

Stats* Stats::getInstance() {
    static Stats stats;
    return &stats;
}

Stats::Stats():
        _expanded(false),
        _recentMaxPackets(0),
        _resetRecentMaxPacketsSoon(true),
        _generalStatsWidth(STATS_GENERAL_MIN_WIDTH),
        _pingStatsWidth(STATS_PING_MIN_WIDTH),
        _geoStatsWidth(STATS_GEO_MIN_WIDTH),
        _voxelStatsWidth(STATS_VOXEL_MIN_WIDTH),
        _lastHorizontalOffset(0)
{
    QGLWidget* glWidget = Application::getInstance()->getGLWidget();
    resetWidth(glWidget->width(), 0);
}

void Stats::toggleExpanded() {
    _expanded = !_expanded;
}

// called on mouse click release
// check for clicks over stats  in order to expand or contract them
void Stats::checkClick(int mouseX, int mouseY, int mouseDragStartedX, int mouseDragStartedY, int horizontalOffset) {
    QGLWidget* glWidget = Application::getInstance()->getGLWidget();

    if (0 != glm::compMax(glm::abs(glm::ivec2(mouseX - mouseDragStartedX, mouseY - mouseDragStartedY)))) {
        // not worried about dragging on stats
        return;
    }

    int statsHeight = 0, 
        statsWidth = 0, 
        statsX = 0, 
        statsY = 0, 
        lines = 0;

    statsX = horizontalOffset;

    // top-left stats click
    lines = _expanded ? 5 : 3;
    statsHeight = lines * STATS_PELS_PER_LINE + 10;
    if (mouseX > statsX && mouseX < statsX + _generalStatsWidth && mouseY > statsY && mouseY < statsY + statsHeight) {
        toggleExpanded();
        return;
    }
    statsX += _generalStatsWidth;

    // ping stats click
    if (Menu::getInstance()->isOptionChecked(MenuOption::TestPing)) {
        lines = _expanded ? 4 : 3;
        statsHeight = lines * STATS_PELS_PER_LINE + 10;
        if (mouseX > statsX && mouseX < statsX + _pingStatsWidth && mouseY > statsY && mouseY < statsY + statsHeight) {
            toggleExpanded();
            return;
        }
        statsX += _pingStatsWidth;
    }

    // geo stats panel click
    lines = _expanded ? 4 : 3;
    statsHeight = lines * STATS_PELS_PER_LINE + 10;
    if (mouseX > statsX && mouseX < statsX + _geoStatsWidth  && mouseY > statsY && mouseY < statsY + statsHeight) {
        toggleExpanded();
        return;
    }
    statsX += _geoStatsWidth;

    // top-right stats click
    lines = _expanded ? 11 : 3;
    statsHeight = lines * STATS_PELS_PER_LINE + 10;
    statsWidth = glWidget->width() - statsX;
    if (mouseX > statsX && mouseX < statsX + statsWidth  && mouseY > statsY && mouseY < statsY + statsHeight) {
        toggleExpanded();
        return;
    }
}

void Stats::resetWidth(int width, int horizontalOffset) {
    QGLWidget* glWidget = Application::getInstance()->getGLWidget();
    int extraSpace = glWidget->width() - horizontalOffset -2
                   - STATS_GENERAL_MIN_WIDTH
                   - (Menu::getInstance()->isOptionChecked(MenuOption::TestPing) ? STATS_PING_MIN_WIDTH -1 : 0)
                   - STATS_GEO_MIN_WIDTH
                   - STATS_VOXEL_MIN_WIDTH;

    int panels = 4;

    _generalStatsWidth = STATS_GENERAL_MIN_WIDTH;
    if (Menu::getInstance()->isOptionChecked(MenuOption::TestPing)) {
        _pingStatsWidth = STATS_PING_MIN_WIDTH;
    } else {
        _pingStatsWidth = 0;
        panels = 3;
    }
    _geoStatsWidth = STATS_GEO_MIN_WIDTH;
    _voxelStatsWidth = STATS_VOXEL_MIN_WIDTH;

    if (extraSpace > panels) {
        _generalStatsWidth += (int) extraSpace / panels;
        if (Menu::getInstance()->isOptionChecked(MenuOption::TestPing)) {
            _pingStatsWidth += (int) extraSpace / panels;
        }
        _geoStatsWidth += (int) extraSpace / panels;
        _voxelStatsWidth += glWidget->width() - (_generalStatsWidth + _pingStatsWidth + _geoStatsWidth + 3);
    }
}


// translucent background box that makes stats more readable
void Stats::drawBackground(unsigned int rgba, int x, int y, int width, int height) {
    glBegin(GL_QUADS);
    glColor4f(((rgba >> 24) & 0xff) / 255.0f,
              ((rgba >> 16) & 0xff) / 255.0f, 
              ((rgba >> 8) & 0xff)  / 255.0f,
              (rgba & 0xff) / 255.0f);
    glVertex3f(x, y, 0);
    glVertex3f(x + width, y, 0);
    glVertex3f(x + width, y + height, 0);
    glVertex3f(x , y + height, 0);
    glEnd();
    glColor4f(1, 1, 1, 1); 
}

// display expanded or contracted stats
void Stats::display(
        const float* color, 
        int horizontalOffset, 
        float fps, 
        int packetsPerSecond, 
        int bytesPerSecond, 
        int voxelPacketsToProcess) 
{
    QGLWidget* glWidget = Application::getInstance()->getGLWidget();

    unsigned int backgroundColor = 0x33333399;
    int verticalOffset = 0, lines = 0;

    QLocale locale(QLocale::English);
    std::stringstream voxelStats;

    if (_lastHorizontalOffset != horizontalOffset) {
        resetWidth(glWidget->width(), horizontalOffset);
        _lastHorizontalOffset = horizontalOffset;
    }

    glPointSize(1.0f);

    // we need to take one avatar out so we don't include ourselves
    int totalAvatars = Application::getInstance()->getAvatarManager().size() - 1;
    int totalServers = NodeList::getInstance()->size();

    lines = _expanded ? 5 : 3;
    drawBackground(backgroundColor, horizontalOffset, 0, _generalStatsWidth, lines * STATS_PELS_PER_LINE + 10);
    horizontalOffset += 5;

    char serverNodes[30];
    sprintf(serverNodes, "Servers: %d", totalServers);
    char avatarNodes[30];
    sprintf(avatarNodes, "Avatars: %d", totalAvatars);
    char framesPerSecond[30];
    sprintf(framesPerSecond, "Framerate: %3.0f FPS", fps);
 
    verticalOffset += STATS_PELS_PER_LINE;
    drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, serverNodes, color);
    verticalOffset += STATS_PELS_PER_LINE;
    drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, avatarNodes, color);
    verticalOffset += STATS_PELS_PER_LINE;
    drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, framesPerSecond, color);

    if (_expanded) {
        char packetsPerSecondString[30];
        sprintf(packetsPerSecondString, "Pkts/sec: %d", packetsPerSecond);
        char averageMegabitsPerSecond[30];
        sprintf(averageMegabitsPerSecond, "Mbps: %3.2f", (float)bytesPerSecond * 8.f / 1000000.f);

        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, packetsPerSecondString, color);
        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, averageMegabitsPerSecond, color);
    }

    verticalOffset = 0;
    horizontalOffset = _lastHorizontalOffset + _generalStatsWidth +1;

    if (Menu::getInstance()->isOptionChecked(MenuOption::TestPing)) {
        int pingAudio = 0, pingAvatar = 0, pingVoxel = 0, pingVoxelMax = 0;

        NodeList* nodeList = NodeList::getInstance();
        SharedNodePointer audioMixerNode = nodeList->soloNodeOfType(NodeType::AudioMixer);
        SharedNodePointer avatarMixerNode = nodeList->soloNodeOfType(NodeType::AvatarMixer);

        pingAudio = audioMixerNode ? audioMixerNode->getPingMs() : 0;
        pingAvatar = avatarMixerNode ? avatarMixerNode->getPingMs() : 0;

        // Now handle voxel servers, since there could be more than one, we average their ping times
        unsigned long totalPingVoxel = 0;
        int voxelServerCount = 0;

        foreach (const SharedNodePointer& node, nodeList->getNodeHash()) {
            if (node->getType() == NodeType::VoxelServer) {
                totalPingVoxel += node->getPingMs();
                voxelServerCount++;
                if (pingVoxelMax < node->getPingMs()) {
                    pingVoxelMax = node->getPingMs();
                }
            }
        }

        if (voxelServerCount) {
            pingVoxel = totalPingVoxel/voxelServerCount;
        }

        lines = _expanded ? 4 : 3;
        drawBackground(backgroundColor, horizontalOffset, 0, _pingStatsWidth, lines * STATS_PELS_PER_LINE + 10);
        horizontalOffset += 5;

        Audio* audio = Application::getInstance()->getAudio();

        char audioJitter[30];
        sprintf(audioJitter,
                "Buffer msecs %.1f",
                (float) (audio->getNetworkBufferLengthSamplesPerChannel() + (float) audio->getJitterBufferSamples()) /
                (float) audio->getNetworkSampleRate() * 1000.f);
        drawText(30, glWidget->height() - 22, 0.10f, 0.f, 2.f, audioJitter, color);
        
                 
        char audioPing[30];
        sprintf(audioPing, "Audio ping: %d", pingAudio);
       
                 
        char avatarPing[30];
        sprintf(avatarPing, "Avatar ping: %d", pingAvatar);
        char voxelAvgPing[30];
        sprintf(voxelAvgPing, "Voxel avg ping: %d", pingVoxel);

        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, audioPing, color);
        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, avatarPing, color);
        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, voxelAvgPing, color);

        if (_expanded) {
            char voxelMaxPing[30];
            sprintf(voxelMaxPing, "Voxel max ping: %d", pingVoxelMax);

            verticalOffset += STATS_PELS_PER_LINE;
            drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, voxelMaxPing, color);
        }

        verticalOffset = 0;
        horizontalOffset = _lastHorizontalOffset + _generalStatsWidth + _pingStatsWidth + 2;
    }

    MyAvatar* myAvatar = Application::getInstance()->getAvatar();
    glm::vec3 avatarPos = myAvatar->getPosition();

    lines = _expanded ? 5 : 3;
    drawBackground(backgroundColor, horizontalOffset, 0, _geoStatsWidth, lines * STATS_PELS_PER_LINE + 10);
    horizontalOffset += 5;

    char avatarPosition[200];
    sprintf(avatarPosition, "Position: %.1f, %.1f, %.1f", avatarPos.x, avatarPos.y, avatarPos.z);
    char avatarVelocity[30];
    sprintf(avatarVelocity, "Velocity: %.1f", glm::length(myAvatar->getVelocity()));
    char avatarBodyYaw[30];
    sprintf(avatarBodyYaw, "Yaw: %.1f", myAvatar->getBodyYaw());
    char avatarMixerStats[200];

    verticalOffset += STATS_PELS_PER_LINE;
    drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, avatarPosition, color);
    verticalOffset += STATS_PELS_PER_LINE;
    drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, avatarVelocity, color);
    verticalOffset += STATS_PELS_PER_LINE;
    drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, avatarBodyYaw, color);

    if (_expanded) {
        SharedNodePointer avatarMixer = NodeList::getInstance()->soloNodeOfType(NodeType::AvatarMixer);
        if (avatarMixer) {
            sprintf(avatarMixerStats, "Avatar Mixer: %.f kbps, %.f pps",
                    roundf(avatarMixer->getAverageKilobitsPerSecond()),
                    roundf(avatarMixer->getAveragePacketsPerSecond()));
        } else {
            sprintf(avatarMixerStats, "No Avatar Mixer");
        }

        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, avatarMixerStats, color);
        
        stringstream downloads;
        downloads << "Downloads: ";
        foreach (Resource* resource, ResourceCache::getLoadingRequests()) {
            downloads << (int)(resource->getProgress() * 100.0f) << "% ";
        }
        downloads << "(" << ResourceCache::getPendingRequestCount() << " pending)";
        
        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, downloads.str().c_str(), color);
    }

    verticalOffset = 0;
    horizontalOffset = _lastHorizontalOffset + _generalStatsWidth + _pingStatsWidth + _geoStatsWidth + 3;

    VoxelSystem* voxels = Application::getInstance()->getVoxels();

    lines = _expanded ? 12 : 3;
    drawBackground(backgroundColor, horizontalOffset, 0, glWidget->width() - horizontalOffset, lines * STATS_PELS_PER_LINE + 10);
    horizontalOffset += 5;

    if (_expanded) {
        // Local Voxel Memory Usage
        voxelStats.str("");
        voxelStats << "Voxels Memory Nodes: " << VoxelTreeElement::getTotalMemoryUsage() / 1000000.f << "MB";
        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, (char*)voxelStats.str().c_str(), color);

        voxelStats.str("");
        voxelStats << 
                "Geometry RAM: " << voxels->getVoxelMemoryUsageRAM() / 1000000.f << "MB / " <<
                "VBO: " << voxels->getVoxelMemoryUsageVBO() / 1000000.f << "MB";
        if (voxels->hasVoxelMemoryUsageGPU()) {
            voxelStats << " / GPU: " << voxels->getVoxelMemoryUsageGPU() / 1000000.f << "MB";
        }
        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, (char*)voxelStats.str().c_str(), color);

        // Voxel Rendering
        voxelStats.str("");
        voxelStats.precision(4);
        voxelStats << "Voxel Rendering Slots Max: " << voxels->getMaxVoxels() / 1000.f << "K";
        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, (char*)voxelStats.str().c_str(), color);
    }

    voxelStats.str("");
    voxelStats.precision(4);
    voxelStats << "Drawn: " << voxels->getVoxelsWritten() / 1000.f << "K " <<
        "Abandoned: " << voxels->getAbandonedVoxels() / 1000.f << "K ";
    verticalOffset += STATS_PELS_PER_LINE;
    drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, (char*)voxelStats.str().c_str(), color);

    // iterate all the current voxel stats, and list their sending modes, and total voxel counts
    std::stringstream sendingMode("");
    sendingMode << "Octree Sending Mode: [";
    int serverCount = 0;
    int movingServerCount = 0;
    unsigned long totalNodes = 0;
    unsigned long totalInternal = 0;
    unsigned long totalLeaves = 0;
    NodeToOctreeSceneStats* octreeServerSceneStats = Application::getInstance()->getOcteeSceneStats();
    for(NodeToOctreeSceneStatsIterator i = octreeServerSceneStats->begin(); i != octreeServerSceneStats->end(); i++) {
        //const QUuid& uuid = i->first;
        OctreeSceneStats& stats = i->second;
        serverCount++;
        if (_expanded) {
            if (serverCount > 1) {
                sendingMode << ",";
            }
            if (stats.isMoving()) {
                sendingMode << "M";
                movingServerCount++;
            } else {
                sendingMode << "S";
            }
        }

        // calculate server node totals
        totalNodes += stats.getTotalElements();
        if (_expanded) {
            totalInternal += stats.getTotalInternal();
            totalLeaves += stats.getTotalLeaves();                
        }
    }
    if (_expanded) {
        if (serverCount == 0) {
            sendingMode << "---";
        }
        sendingMode << "] " << serverCount << " servers";
        if (movingServerCount > 0) {
            sendingMode << " <SCENE NOT STABLE>";
        } else {
            sendingMode << " <SCENE STABLE>";
        }
        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, (char*)sendingMode.str().c_str(), color);
    }

    // Incoming packets
    if (_expanded) {
        voxelStats.str("");
        QString packetsString = locale.toString((int)voxelPacketsToProcess);
        QString maxString = locale.toString((int)_recentMaxPackets);
        voxelStats << "Voxel Packets to Process: " << qPrintable(packetsString)
                    << " [Recent Max: " << qPrintable(maxString) << "]";        
        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, (char*)voxelStats.str().c_str(), color);
    }

    if (_resetRecentMaxPacketsSoon && voxelPacketsToProcess > 0) {
        _recentMaxPackets = 0;
        _resetRecentMaxPacketsSoon = false;
    }
    if (voxelPacketsToProcess == 0) {
        _resetRecentMaxPacketsSoon = true;
    } else {
        if (voxelPacketsToProcess > _recentMaxPackets) {
            _recentMaxPackets = voxelPacketsToProcess;
        }
    }

    verticalOffset += (_expanded ? STATS_PELS_PER_LINE : 0);

    QString serversTotalString = locale.toString((uint)totalNodes); // consider adding: .rightJustified(10, ' ');

    // Server Voxels
    voxelStats.str("");
    voxelStats << "Server voxels: " << qPrintable(serversTotalString);
    verticalOffset += STATS_PELS_PER_LINE;
    drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, (char*)voxelStats.str().c_str(), color);

    if (_expanded) {
        QString serversInternalString = locale.toString((uint)totalInternal);
        QString serversLeavesString = locale.toString((uint)totalLeaves);

        voxelStats.str("");
        voxelStats <<
            "Internal: " << qPrintable(serversInternalString) << "  " <<
            "Leaves: " << qPrintable(serversLeavesString) << "";
        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, (char*)voxelStats.str().c_str(), color);
    }

    unsigned long localTotal = VoxelTreeElement::getNodeCount();
    QString localTotalString = locale.toString((uint)localTotal); // consider adding: .rightJustified(10, ' ');

    // Local Voxels
    voxelStats.str("");
    voxelStats << "Local voxels: " << qPrintable(localTotalString);
    verticalOffset += STATS_PELS_PER_LINE;
    drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, (char*)voxelStats.str().c_str(), color);

    if (_expanded) {
        unsigned long localInternal = VoxelTreeElement::getInternalNodeCount();
        unsigned long localLeaves = VoxelTreeElement::getLeafNodeCount();
        QString localInternalString = locale.toString((uint)localInternal);
        QString localLeavesString = locale.toString((uint)localLeaves);

        voxelStats.str("");
        voxelStats <<
            "Internal: " << qPrintable(localInternalString) << "  " <<
            "Leaves: " << qPrintable(localLeavesString) << "";
        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0, 2, (char*)voxelStats.str().c_str(), color);
    }

    // LOD Details
    if (_expanded) {
        voxelStats.str("");
        QString displayLODDetails = Menu::getInstance()->getLODFeedbackText();
        voxelStats << "LOD: You can see " << qPrintable(displayLODDetails.trimmed());
        verticalOffset += STATS_PELS_PER_LINE;
        drawText(horizontalOffset, verticalOffset, 0.10f, 0.f, 2.f, (char*)voxelStats.str().c_str(), color);
    }    
}
