#include "Item/LifeMaxUpItem.h"
#include "Library/Demo/DemoFunction.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Util/ItemUtil.h"
#include "Util/SensorMsgFunction.h"

#include "LifeMaxUpItem.h"
#include "LifeUpItem.h"

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

    mFlashingCtrlDither->end();
    mFromAmiibo = false;
    mCoinValue = 10;
}

void LifeMaxUpItem::control() {
    if (al::isNerve(this, &NrvLifeMaxUpItem.Appeared) ||
        al::isNerve(this, &NrvLifeMaxUpItem.StayPlacedPos) ||
        al::isNerve(this, &NrvLifeMaxUpItem.WaterFallWorld)) {
        mIntangibilityTimer = sead::Mathi::clampMin(mIntangibilityTimer - 1, 0);
        mWaterTimer = sead::Mathi::clampMin(mWaterTimer - 1, 0);

        mAngle = 0 + al::modf(mAngle + mSpinSpeed + 360.0f, 360.0f);
    }
}

void LifeMaxUpItem::appearAmiiboTouch(const sead::Vector3f& position) {
    appear();
    mCoinValue = 0;
    mFromAmiibo = true;
    mFlashingCtrlDither->start(60);

    al::resetPosition(this, position);
    al::startAction(this, "AppearAbove");
    al::startHitReaction(this, "Amiibo");
    al::setNerve(this, &NrvLifeMaxUpItem.AutoGetDemo);
}

void LifeMaxUpItem::appearPopUp() {
    appear();

    sead::Vector3f front_direction{0, 0, 0};
    al::calcFrontDir(&front_direction, this);

    sead::Vector3f goal = front_direction * 3.1f;
    sead::Vector3f gravity = al::getGravity(this);

    sead::Vector3f velocity = goal - gravity * 11;

    al::setVelocity(this, velocity);
    al::startAction(this, "AppearPopUp");
    al::startHitReaction(this, "飛出し出現");
    al::setNerve(this, &NrvLifeMaxUpItem.Appeared);
}

void LifeMaxUpItem::appearPopUpAbove() {
    appear();

    sead::Vector3f front_direction{0, 0, 0};
    al::calcFrontDir(&front_direction, this);
    sead::Vector3f gravity = al::getGravity(this);
    sead::Vector3f inverseGravity = -gravity;

    sead::Quatf pose{};
    al::makeQuatUpFront(&pose, inverseGravity, front_direction);
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

    sead::Vector3f front_direction{0, 0, 0};
    al::calcFrontDir(&front_direction, this);

    sead::Vector3f goal = front_direction * 3.1f;

    sead::Vector3f gravity = al::getGravity(this);

    sead::Vector3f velocity = goal - gravity * 11;

    al::setVelocity(this, velocity);
    al::startAction(this, "AppearPopUp");
    al::startHitReaction(this, "スロット出現");
    al::setNerve(this, &NrvLifeMaxUpItem.Appeared);
}

void LifeMaxUpItem::appearPopUpDir(const sead::Quatf& dir) {
    appear();

    sead::Vector3f front_direction{0, 0, 0};
    al::setQuat(this, dir);
    al::calcFrontDir(&front_direction, this);

    sead::Vector3f goal = front_direction * 3.1f;

    sead::Vector3f gravity = al::getGravity(this);

    sead::Vector3f velocity = goal - gravity * 11;

    al::setVelocity(this, velocity);
    al::startAction(this, "AppearPopUp");
    al::startHitReaction(this, "スロット出現");
    al::setNerve(this, &NrvLifeMaxUpItem.Appeared);
}

void LifeMaxUpItem::exeAppeared() {
    if (al::isCollidedFloorCode(this, "Poison")) {
        al::startHitReaction(this, "死亡");
        this->kill();
        return;
    }

    if (al::isInDeathArea(this) && mFlashingCtrlDither->time > 300) {
        mFlashingCtrlDither->start(300);
        return;
    }

    sead::Vector3f translation = al::getTrans(this);

    sead::Vector3f check_pos = translation + sead::Vector3f(0, 130.0f, 0);
    
    bool is_upper_in_water = al::isInWaterPos(this, check_pos);
    sead::Vector3f translationAgain = al::getTrans(this);

    check_pos = translationAgain + sead::Vector3f(0, 15.0f, 0);

    bool is_lower_in_water = al::isInWaterPos(this, check_pos);

    if (LifeUpItemFunction::updateSurfaceFinder(this->mWaterSurfaceFinder, this, .4f) ||
        LifeUpItemFunction::updateSurfaceFinder(this->mFireSurfaceFinder, this, .4f)) {
        LifeUpItemFunction::updateVelocityInWaterAndFire(this, this->mWaterSurfaceFinder,
                                                         this->mFireSurfaceFinder, 50.f, .4f);
        tryStartPlayingAnimation(this, "Float");
    } else if (is_upper_in_water || is_lower_in_water) {
        al::addVelocityToGravityLimit(this, -.15f, 8.5f);
        al::tryStartActionIfNotPlaying(this, "Float");
        return;
    } else {
        al::addVelocityToGravityLimit(this, .4f, 20.f);
        tryStartPlayingAnimation(this, "Wait");
    }

    if (!is_upper_in_water && is_lower_in_water && mWaterTimer == 0)
        al::startAction(this, "LandFloat");

    if (is_upper_in_water || is_lower_in_water)
        mWaterTimer = 20;

    if (al::isGreaterEqualStep(this, 3)) {
        if (al::isOnGround(this, 0)) {
            al::scaleVelocityHV(this, .5f, 0.f);
            if (this->mWaterTimer == 0)
                al::startAction(this, "Land");
            this->mWaterTimer = 20;
        }

        if (al::isCollidedCeilingVelocity(this))
            al::scaleVelocityHV(this, 1.f, .8f);

        if (al::isCollidedWallVelocity(this))
            al::scaleVelocityHV(this, .5f, 1.f);
    }

    float rotationSpeed;
    float goalSpeed;

    if(al::isActionPlaying(this, "Wait") || al::isActionPlaying(this, "Land")) {
        rotationSpeed = mSpinSpeed;
        goalSpeed = 2.0f;
    }else {
        rotationSpeed = mSpinSpeed;
        goalSpeed = 6.0f;
    }

    rotationSpeed = al::lerpValue(rotationSpeed, goalSpeed, 0.035f);
    mSpinSpeed = rotationSpeed;
    return;
}

void LifeMaxUpItem::exeGotAppearCoin() {
    if (this->mCoinValue > 0) {
        al::HitSensor* sensor = al::getHitSensor(this, nullptr);
        s32 step = al::getNerveStep(this);

        if (rs::tryAppearMultiCoinFromObj(this, sensor, step, 150.0f))
            mCoinValue -= 1;

        if (mCoinValue > 0)
            return;
    }

    al::setNerve(this, &NrvLifeMaxUpItem.GotDeadWait);
}

void tryStartPlayingAnimation(al::LiveActor* actor, const char* name) {
    if (!al::isActionPlaying(actor, name) &&
        (!al::isActionPlaying(actor, "Land") || al::isActionEnd(actor)) &&
        (!al::isActionPlaying(actor, "LandFloat") || al::isActionEnd(actor)) &&
        !al::isActionPlaying(actor, "AppearPopUp") && !al::isActionPlaying(actor, "AppearAbove")) {
        al::startAction(actor, name);
    }
}
