//
//  AvatarManager.cpp
//  interface/src/avatar
//
//  Created by Stephen Birarda on 1/23/2014.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <string>

#include <glm/gtx/string_cast.hpp>

#include <PerfStat.h>
#include <UUID.h>

#include "Application.h"
#include "Avatar.h"
#include "Menu.h"
#include "MyAvatar.h"

#include "AvatarManager.h"

// We add _myAvatar into the hash with all the other AvatarData, and we use the default NULL QUid as the key.
const QUuid MY_AVATAR_KEY;  // NULL key

AvatarManager::AvatarManager(QObject* parent) :
    _avatarFades() {
    // register a meta type for the weak pointer we'll use for the owning avatar mixer for each avatar
    qRegisterMetaType<QWeakPointer<Node> >("NodeWeakPointer");
    _myAvatar = QSharedPointer<MyAvatar>(new MyAvatar());
}

void AvatarManager::init() {
    _myAvatar->init();
    _myAvatar->setPosition(START_LOCATION);
    _myAvatar->setDisplayingLookatVectors(false);
    _avatarHash.insert(MY_AVATAR_KEY, _myAvatar);
}

void AvatarManager::updateOtherAvatars(float deltaTime) {
    bool showWarnings = Menu::getInstance()->isOptionChecked(MenuOption::PipelineWarnings);
    PerformanceWarning warn(showWarnings, "Application::updateAvatars()");

    Application* applicationInstance = Application::getInstance();
    glm::vec3 mouseOrigin = applicationInstance->getMouseRayOrigin();
    glm::vec3 mouseDirection = applicationInstance->getMouseRayDirection();

    // simulate avatars
    AvatarHash::iterator avatarIterator = _avatarHash.begin();
    while (avatarIterator != _avatarHash.end()) {
        Avatar* avatar = static_cast<Avatar*>(avatarIterator.value().data());
        if (avatar == static_cast<Avatar*>(_myAvatar.data()) || !avatar->isInitialized()) {
            // DO NOT update _myAvatar!  Its update has already been done earlier in the main loop.
            // DO NOT update uninitialized Avatars
            ++avatarIterator;
            continue;
        }
        if (avatar->getOwningAvatarMixer()) {
            // this avatar's mixer is still around, go ahead and simulate it
            avatar->simulate(deltaTime);
            avatar->setMouseRay(mouseOrigin, mouseDirection);
            ++avatarIterator;
        } else {
            // the mixer that owned this avatar is gone, give it to the vector of fades and kill it
            avatarIterator = erase(avatarIterator);
        }
    }
    
    // simulate avatar fades
    simulateAvatarFades(deltaTime);
}

void AvatarManager::renderAvatars(Avatar::RenderMode renderMode, bool selfAvatarOnly) {
    PerformanceWarning warn(Menu::getInstance()->isOptionChecked(MenuOption::PipelineWarnings),
                            "Application::renderAvatars()");
    bool renderLookAtVectors = Menu::getInstance()->isOptionChecked(MenuOption::LookAtVectors);
    
    glm::vec3 cameraPosition = Application::getInstance()->getCamera()->getPosition();

    if (!selfAvatarOnly) {
        foreach (const AvatarSharedPointer& avatarPointer, _avatarHash) {
            Avatar* avatar = static_cast<Avatar*>(avatarPointer.data());
            if (!avatar->isInitialized()) {
                continue;
            }
            avatar->render(cameraPosition, renderMode);
            avatar->setDisplayingLookatVectors(renderLookAtVectors);
        }
        renderAvatarFades(cameraPosition, renderMode);
    } else {
        // just render myAvatar
        _myAvatar->render(cameraPosition, renderMode);
        _myAvatar->setDisplayingLookatVectors(renderLookAtVectors);
    }
}

void AvatarManager::simulateAvatarFades(float deltaTime) {
    QVector<AvatarSharedPointer>::iterator fadingIterator = _avatarFades.begin();
    
    const float SHRINK_RATE = 0.9f;
    const float MIN_FADE_SCALE = 0.001f;

    while (fadingIterator != _avatarFades.end()) {
        Avatar* avatar = static_cast<Avatar*>(fadingIterator->data());
        avatar->setTargetScale(avatar->getScale() * SHRINK_RATE);
        if (avatar->getTargetScale() < MIN_FADE_SCALE) {
            fadingIterator = _avatarFades.erase(fadingIterator);
        } else {
            avatar->simulate(deltaTime);
            ++fadingIterator;
        }
    }
}

void AvatarManager::renderAvatarFades(const glm::vec3& cameraPosition, Avatar::RenderMode renderMode) {
    // render avatar fades
    Glower glower(renderMode == Avatar::NORMAL_RENDER_MODE ? 1.0f : 0.0f);
    
    foreach(const AvatarSharedPointer& fadingAvatar, _avatarFades) {
        Avatar* avatar = static_cast<Avatar*>(fadingAvatar.data());
        if (avatar != static_cast<Avatar*>(_myAvatar.data()) && avatar->isInitialized()) {
            avatar->render(cameraPosition, renderMode);
        }
    }
}

AvatarSharedPointer AvatarManager::matchingOrNewAvatar(const QUuid& sessionUUID, const QWeakPointer<Node>& mixerWeakPointer) {
    AvatarSharedPointer matchingAvatar = _avatarHash.value(sessionUUID);
    
    if (!matchingAvatar) {
        // construct a new Avatar for this node
        Avatar* avatar =  new Avatar();
        avatar->setOwningAvatarMixer(mixerWeakPointer);
        
        // insert the new avatar into our hash
        matchingAvatar = AvatarSharedPointer(avatar);
        _avatarHash.insert(sessionUUID, matchingAvatar);
        
        qDebug() << "Adding avatar with sessionUUID " << sessionUUID << "to AvatarManager hash.";
    }
    
    return matchingAvatar;
}

void AvatarManager::processAvatarMixerDatagram(const QByteArray& datagram, const QWeakPointer<Node>& mixerWeakPointer) {
    switch (packetTypeForPacket(datagram)) {
        case PacketTypeBulkAvatarData:
            processAvatarDataPacket(datagram, mixerWeakPointer);
            break;
        case PacketTypeAvatarIdentity:
            processAvatarIdentityPacket(datagram, mixerWeakPointer);
            break;
        case PacketTypeAvatarBillboard:
            processAvatarBillboardPacket(datagram, mixerWeakPointer);
            break;
        case PacketTypeKillAvatar:
            processKillAvatar(datagram);
            break;
        default:
            break;
    }
}

void AvatarManager::processAvatarDataPacket(const QByteArray &datagram, const QWeakPointer<Node> &mixerWeakPointer) {
    int bytesRead = numBytesForPacketHeader(datagram);
    
    // enumerate over all of the avatars in this packet
    // only add them if mixerWeakPointer points to something (meaning that mixer is still around)
    while (bytesRead < datagram.size() && mixerWeakPointer.data()) {
        QUuid sessionUUID = QUuid::fromRfc4122(datagram.mid(bytesRead, NUM_BYTES_RFC4122_UUID));
        bytesRead += NUM_BYTES_RFC4122_UUID;
        
        AvatarSharedPointer matchingAvatarData = matchingOrNewAvatar(sessionUUID, mixerWeakPointer);
        
        // have the matching (or new) avatar parse the data from the packet
        bytesRead += matchingAvatarData->parseDataAtOffset(datagram, bytesRead);
        
        Avatar* matchingAvatar = reinterpret_cast<Avatar*>(matchingAvatarData.data());
        
        if (!matchingAvatar->isInitialized()) {
            // now that we have AvatarData for this Avatar we are go for init
            matchingAvatar->init();
        }
    }
}

void AvatarManager::processAvatarIdentityPacket(const QByteArray &packet, const QWeakPointer<Node>& mixerWeakPointer) {
    // setup a data stream to parse the packet
    QDataStream identityStream(packet);
    identityStream.skipRawData(numBytesForPacketHeader(packet));
    
    QUuid sessionUUID;
    
    while (!identityStream.atEnd()) {
        
        QUrl faceMeshURL, skeletonURL;
        QString displayName;
        identityStream >> sessionUUID >> faceMeshURL >> skeletonURL >> displayName;

        // mesh URL for a UUID, find avatar in our list
        AvatarSharedPointer matchingAvatar = matchingOrNewAvatar(sessionUUID, mixerWeakPointer);
        if (matchingAvatar) {
            Avatar* avatar = static_cast<Avatar*>(matchingAvatar.data());
            
            if (avatar->getFaceModelURL() != faceMeshURL) {
                avatar->setFaceModelURL(faceMeshURL);
            }
            
            if (avatar->getSkeletonModelURL() != skeletonURL) {
                avatar->setSkeletonModelURL(skeletonURL);
            }

            if (avatar->getDisplayName() != displayName) {
                avatar->setDisplayName(displayName);
            }
        }
    }
}

void AvatarManager::processAvatarBillboardPacket(const QByteArray& packet, const QWeakPointer<Node>& mixerWeakPointer) {
    int headerSize = numBytesForPacketHeader(packet);
    QUuid sessionUUID = QUuid::fromRfc4122(QByteArray::fromRawData(packet.constData() + headerSize, NUM_BYTES_RFC4122_UUID));
    
    AvatarSharedPointer matchingAvatar = matchingOrNewAvatar(sessionUUID, mixerWeakPointer);
    if (matchingAvatar) {
        Avatar* avatar = static_cast<Avatar*>(matchingAvatar.data());
        QByteArray billboard = packet.mid(headerSize + NUM_BYTES_RFC4122_UUID);
        if (avatar->getBillboard() != billboard) {
            avatar->setBillboard(billboard);
        }
    }
}

void AvatarManager::processKillAvatar(const QByteArray& datagram) {
    // read the node id
    QUuid sessionUUID = QUuid::fromRfc4122(datagram.mid(numBytesForPacketHeader(datagram), NUM_BYTES_RFC4122_UUID));
    
    // remove the avatar with that UUID from our hash, if it exists
    AvatarHash::iterator matchedAvatar = _avatarHash.find(sessionUUID);
    if (matchedAvatar != _avatarHash.end()) {
        erase(matchedAvatar);
    }
}

AvatarHash::iterator AvatarManager::erase(const AvatarHash::iterator& iterator) {
    if (iterator.key() != MY_AVATAR_KEY) {
        qDebug() << "Removing Avatar with UUID" << iterator.key() << "from AvatarManager hash.";
        if (reinterpret_cast<Avatar*>(iterator.value().data())->isInitialized()) {
            _avatarFades.push_back(iterator.value());
        }
        return AvatarHashMap::erase(iterator);
    } else {
        // never remove _myAvatar from the list
        AvatarHash::iterator returnIterator = iterator;
        return ++returnIterator;
    }
}

void AvatarManager::clearOtherAvatars() {
    // clear any avatars that came from an avatar-mixer
    AvatarHash::iterator removeAvatar =  _avatarHash.begin();
    while (removeAvatar != _avatarHash.end()) {
        removeAvatar = erase(removeAvatar);
    }
    _myAvatar->clearLookAtTargetAvatar();
}
