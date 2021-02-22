#ifndef MINECRAFT_H
#define MINECRAFT_H

#include <Arduino.h>

class minecraft{
    public:

    class player{

        public:
        Stream* S;

        bool connected = false;
        String username;
        double x;
        double y;
        double z;
        double yaw;
        double pitch;
        float health = 0;
        uint8_t food = 0;
        float food_sat = 0;
        uint8_t id = 0;

        bool join               ();
        void handle             ();

        uint8_t readHandShake   ();
        bool readLoginStart     ();
        uint64_t readPing       ();
        void readRequest        ();

        void writeResponse      ();
        void writeLoginSuccess  ();
        void writeChunk         ();
        void writePlayerPositionAndLook(double x, double y, double z, float yaw, float pitch, uint8_t flags);
        void writePlayerInfo    ();
        void writeKeepAlive     ();
        void writeServerDifficulty();
        void writeSpawnPlayer   ();
        void writeJoinGame      ();
        void writePong          (uint64_t payload);
        void writeSubChunk      (uint8_t index);

        void loginfo            (String msg);
        void logerr             (String msg);

        float readFloat         ();
        double readDouble       ();
        int32_t readVarInt      ();
        String readString       ();
        int64_t readLong        ();
        uint16_t readUnsignedShort();
        uint32_t VarIntLength   (int val);

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

    uint64_t tick = 0;
    uint64_t prev_keepalive = 0;

    void handle();
    player players[5];
};

int32_t lsr(int32_t x, uint32_t n);

#endif