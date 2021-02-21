#ifndef MINECRAFT_H
#define MINECRAFT_H

#include <Arduino.h>

class minecraft{
    String username;

    public:
        minecraft(Stream* __S, uint8_t _player_num);

        Stream* S;
        bool writing = 0;
        double x = 0;
        double y = 0;
        double z = 0;
        float yaw = 0;
        float pitch = 0;
        bool onGround = true;
        bool teleported = false;
        float health = 0;
        uint8_t food = 0;
        float food_sat = 0;
        uint64_t prev_keepalive = 0;
        uint8_t player_num = 0;

        int timeout = 100;

        void loginfo            (String msg);
        void logerr             (String msg);

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

        float readFloat         ();
        double readDouble       ();
        int32_t readVarInt     ();
        String readString       ();
        int64_t readLong       ();
        uint16_t readUnsignedShort();
        uint32_t VarIntLength   (int val);

    private:
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

        void writeSubChunk      (uint8_t index);
};

int32_t lsr(int32_t x, uint32_t n);

#endif