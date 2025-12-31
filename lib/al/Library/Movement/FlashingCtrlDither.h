namespace al {

class FlashingCtrlDither {
public:
    void end();
    void start(int time);

private:
    char pad[0x14];
public:
    int time;
};

}  // namespace al
