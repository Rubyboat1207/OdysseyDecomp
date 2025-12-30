#include "Item/LifeMaxUpItem.h"
#include "Library/Demo/DemoFunction.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Util/ItemUtil.h"
#include "Util/SensorMsgFunction.h"

#include "LifeMaxUpItem.h"

namespace {
NERVE_IMPL(LifeMaxUpItem, WaterFallWorld);
NERVE_IMPL(LifeMaxUpItem, StayPlacedPos);
NERVE_IMPL(LifeMaxUpItem, Appeared);
NERVE_IMPL(LifeMaxUpItem, AutoGetDemo);
NERVE_IMPL(LifeMaxUpItem, GotWaitLifeUpDemo);
NERVE_IMPL(LifeMaxUpItem, GotAppearCoin);
NERVE_IMPL(LifeMaxUpItem, GotDeadWait);

NERVES_MAKE_STRUCT(LifeMaxUpItem, WaterFallWorld, StayPlacedPos, Appeared, AutoGetDemo,
                   GotWaitLifeUpDemo, GotAppearCoin, GotDeadWait);
}  // namespace

void LifeMaxUpItem::appear() {
    al::LiveActor::appear();
    al::showModelIfHide(this);
    al::validateClipping(this);
    al::validateHitSensors(this);
    al::onCollide(this);

    flashingCtrlDither->end();
    fromAmiibo = false;
    coinValue = 10;
}

void LifeMaxUpItem::control() {
    if (al::isNerve(this, &NrvLifeMaxUpItem.Appeared) ||
        al::isNerve(this, &NrvLifeMaxUpItem.StayPlacedPos) ||
        al::isNerve(this, &NrvLifeMaxUpItem.WaterFallWorld)) {
        
        intangibilityTimer = sead::Mathi::clampMin(intangibilityTimer - 1, 0);
        waterTimer = sead::Mathi::clampMin(waterTimer - 1, 0);

        angle = 0 + al::modf(angle + spinSpeed + 360.0f, 360.0f);
    }
}

void LifeMaxUpItem::appearAmiiboTouch(const sead::Vector3f& position) {
    appear();
    coinValue = 0;
    fromAmiibo = true;
    flashingCtrlDither->start(60);

    al::resetPosition(this, position);
    al::startAction(this, "AppearAbove");
    al::startHitReaction(this, "Amiibo");
    al::setNerve(this, &NrvLifeMaxUpItem.AutoGetDemo);
}

void LifeMaxUpItem::appearPopUp() {
    appear();

    sead::Vector3f frontDir{0, 0, 0};
    al::calcFrontDir(&frontDir, this);

    sead::Vector3f goal = frontDir * 3.1f;
    sead::Vector3f gravity = al::getGravity(this);

    sead::Vector3f velocity = goal - gravity * 11;

    al::setVelocity(this, velocity);
    al::startAction(this, "AppearPopUp");
    al::startHitReaction(this, "飛出し出現");
    al::setNerve(this, &NrvLifeMaxUpItem.Appeared);
}

void LifeMaxUpItem::appearPopUpAbove() {
    appear();

    sead::Vector3f frontDir{0, 0, 0};
    al::calcFrontDir(&frontDir, this);
    sead::Vector3f gravity = al::getGravity(this);
    sead::Vector3f inverseGravity = -gravity;

    sead::Quatf pose{};
    al::makeQuatUpFront(&pose, inverseGravity, frontDir);
    al::updatePoseQuat(this, pose);

    gravity = al::getGravity(this);
    sead::Vector3f velocity = gravity * -11.0f;
    al::setVelocity(this, velocity);
    al::startAction(this, "AppearAbove");
    al::startHitReaction(this, "飛出し出現");
    al::setNerve(this, &NrvLifeMaxUpItem.Appeared);
}

void LifeMaxUpItem::appearSlot() {
    appear();

    sead::Vector3f frontDir{0, 0, 0};
    al::calcFrontDir(&frontDir, this);

    sead::Vector3f goal = frontDir * 3.1f;

    sead::Vector3f gravity = al::getGravity(this);

    sead::Vector3f velocity = goal - gravity * 11;

    al::setVelocity(this, velocity);
    al::startAction(this, "AppearPopUp");
    al::startHitReaction(this, "スロット出現");
    al::setNerve(this, &NrvLifeMaxUpItem.Appeared);
}

void LifeMaxUpItem::appearPopUpDir(const sead::Quatf& dir) {
    appear();

    sead::Vector3f frontDir{0, 0, 0};
    al::setQuat(this, dir);
    al::calcFrontDir(&frontDir, this);

    sead::Vector3f goal = frontDir * 3.1f;

    sead::Vector3f gravity = al::getGravity(this);

    sead::Vector3f velocity = goal - gravity * 11;

    al::setVelocity(this, velocity);
    al::startAction(this, "AppearPopUp");
    al::startHitReaction(this, "スロット出現");
    al::setNerve(this, &NrvLifeMaxUpItem.Appeared);
}

void LifeMaxUpItem::exeGotAppearCoin() {
    if (this->coinValue > 0) {
        al::HitSensor* sensor = al::getHitSensor(this, nullptr);
        s32 step = al::getNerveStep(this);

        if (rs::tryAppearMultiCoinFromObj(this, sensor, step, 150.0f))
            coinValue -= 1;

        if (coinValue > 0)
            return;
    }

    al::setNerve(this, &NrvLifeMaxUpItem.GotDeadWait);
}
