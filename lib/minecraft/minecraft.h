#ifndef MINECRAFT_H
#define MINECRAFT_H

#include <Arduino.h>
#include <mutex>
#include <array>

class packet{
    public:
    uint8_t buffer[6000];
    uint32_t index = 0;

    void write(uint8_t val);
    void write(uint8_t * buf, size_t size);

    void writeDouble        (double value);
    void writeFloat         (float value);
    void writeVarInt        (int32_t value);
    void writeVarLong       (int64_t value);
    void writeString        (String str);
    void writeUnsignedLong  (uint64_t num);
    void writeUnsignedShort (uint16_t num);
    void writeUnsignedByte  (uint8_t num);
    void writeLong          (int64_t num);
    void writeInt           (int32_t num);
    void writeShort         (int16_t num);
    void writeByte          (int8_t num);
    void writeBoolean       (uint8_t val);
    void writeUUID          (int user_id);
};

class minecraft{
    public:
    class player{
        public:
        std::mutex * mtx;
        Stream* S;
        minecraft* mc;
        bool connected = false;
        String username;
        double x = 0;
        double y = 5;
        double z = 0;
        double yaw = 0;
        double pitch = 0;
        int yaw_i = 0;
        int pitch_i = 0;
        bool on_ground = true;
        float health = 0;
        uint8_t food = 0;
        float food_sat = 0;
        uint8_t id = 0;

        player(){  // since mutex is neither moveable or copyable we make a new instance in the constructor
            mtx = new std::mutex();
        }

        bool join               ();
        void handle             ();

        uint8_t readHandShake   ();
        bool readLoginStart     ();
        uint64_t readPing       ();
        void readRequest        ();

        void readChat           ();
        void readPosition       ();
        void readRotation       ();
        void readKeepAlive      ();
        void readPositionAndLook();
        void readTeleportConfirm();
        void readAnimation      ();
        void readEntityAction   ();

        void writeResponse      ();
        void writeLoginSuccess  ();
        void writeChunk         ();
        void writePlayerPositionAndLook(double x, double y, double z, float yaw, float pitch, uint8_t flags);
        void writeKeepAlive     ();
        void writeServerDifficulty();
        void writeSpawnPlayer   (double x, double y, double z, int yaw, int pitch, uint8_t id);
        void writeJoinGame      ();
        void writePong          (uint64_t payload);
        void writeSubChunk      (uint8_t index);
        void writeChat          (String msg, String username);
        void writeEntityTeleport(double x, double y, double z, int yaw, int pitch, bool on_ground, uint8_t id);
        void writeEntityRotation(int yaw, int pitch, bool on_ground, uint8_t id);
        void writeEntityLook    (int yaw, uint8_t id);
        void writeEntityAnimation(uint8_t anim, uint8_t id);
        void writeEntityAction  (uint8_t action, uint8_t id);

        void loginfo            (String msg);
        void logerr             (String msg);
        void login              (String msg);
        void logout             (String msg);

        float readFloat         ();
        double readDouble       ();
        int32_t readVarInt      ();
        String readString       ();
        int64_t readLong        ();
        uint16_t readUnsignedShort();
        uint32_t VarIntLength   (int val);
        uint8_t readByte        ();
        bool readBool           ();

        void writeLength        (uint32_t length);
    };

    uint64_t tick = 0;
    uint64_t prev_keepalive = 0;
    player players[5];

    void handle                      ();
    void broadcastChatMessage        (String msg, String username);
    void broadcastSpawnPlayer        ();
    void broadcastPlayerPosAndLook   (double x, double y, double z, int yaw, int pitch, bool on_ground, uint8_t id);
    void broadcastPlayerInfo         ();
    void broadcastPlayerRotation     (int yaw, int pitch, bool on_ground, uint8_t id);
    void broadcastEntityAnimation    (uint8_t anim, uint8_t id);
    void broadcastEntityAction       (uint8_t action, uint8_t id);
    uint8_t getPlayerNum             ();
};

int32_t lsr(int32_t x, uint32_t n);
float fmap(float x, float in_min, float in_max, float out_min, float out_max);

#endif