#ifndef MINECRAFT_H
#define MINECRAFT_H

#include <Arduino.h>

class minecraft{
    String username;

    public:
        minecraft(Stream* __S);

        Stream* S;
        bool compression_enabled = 0;
        int compression_treshold = 0;
        bool writing = 0;
        double x = 0;
        double y = 0;
        double z = 0;
        float yaw = 0;
        float pitch = 0;
        bool onGround = true;
        bool teleported = false;
        float health = 0;
        int food = 0;
        float food_sat = 0;
        long long prev_keepalive = 0;
        
        int timeout = 100;

        bool handle_join        ();
        void handle             ();

        bool readHandShake      ();
        bool readLoginStart     ();

        void writeLoginSuccess  ();
        void writeChunk         ();
        void writePlayerPositionAndLook(double x, double y, double z, float yaw, float pitch, uint8_t flags);
        void writePlayerInfo    ();
        void writeKeepAlive     ();
        void writeServerDifficulty();
        void writeSpawnPlayer   ();
        void writeJoinGame      ();

        float readFloat         ();
        double readDouble       ();
        int readVarInt          ();
        String readString       ();
        long readLong           ();
        int readUnsignedShort   ();
        int VarIntLength        (int val);

    private:
        void writeDouble        (double value);
        void writeFloat         (float value);
        void writeVarInt        (int32_t value);
        void writeVarLong       (int64_t value);
        void writeString        (String str);
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

int lsr(int x, int n);

#endif