#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorInitInfo;
class HitSensor;
struct FlashingCtrlDither {
    void end();
    void start(int time);
};
}  // namespace al

class LifeMaxUpItem : public al::LiveActor {
public:
    LifeMaxUpItem(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void initAfterPlacement() override;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                    al::HitSensor* self) override;
    void appear() override;
    void control() override;

    void appearAmiiboTouch(const sead::Vector3f&);
    void appearPopUp();
    void appearPopUpAbove();
    void appearSlot();
    void appearPopUpDir(const sead::Quatf&);

    void exeAppeared();
    void exeStayPlacedPos();
    void exeWaterFallWorld();
    void exeAutoGetDemo();
    void exeGotWaitLifeUpDemo();
    void exeGotAppearCoin();
    void exeGotDeadWait();

private:
    char pad[0x2];
    al::FlashingCtrlDither* flashingCtrlDither;
    bool fromAmiibo;
    int coinValue;
    int intangibilityTimer;
    char pad2[0x14];
    int waterTimer;
    float spinSpeed;
    float angle;
    char pad3[0x5];
};
