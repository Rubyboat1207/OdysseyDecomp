namespace al {

class FlashingCtrlDither {
public:
    void end();
    FlashingCtrlDither(LiveActor* actor);
    void movement();
    void start(int time);

private:
    char pad[0x14];

public:
    int time;
};

}  // namespace al
