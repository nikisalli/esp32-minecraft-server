#include "minecraft.h"
#include <Arduino.h>
#include <chunk.h>

// public //

minecraft::minecraft(Stream* __S){
    S = __S;
    if(!handle_join()){
        return;
    }
    writeJoinGame();
    writePlayerInfo();
    writePlayerPositionAndLook(0, 5, 0, 5, 0, 0x00);
    // writeSpawnPlayer();
    writeServerDifficulty();
    writeChunk();
}

bool minecraft::readHandShake(){
    readVarInt(); // length
    int id = readVarInt(); // packet id
    int protocol_version = readVarInt();
    readString(); // we don't need our name
    readUnsignedShort();
    int state = readVarInt();
    Serial.println("[INFO] client state = " + String(state));
    if(id != 0){
        return false;
    } else if(protocol_version != 754){
        Serial.println("[ERROR] wrong protocol version, log in with 1.16.5");
        return false;
    }

    if(state != 2){
        Serial.println("[ERROR] wrong state TODO implement status packet");
        return false;
    }
    return true;
}

bool minecraft::readLoginStart(){
    readVarInt(); // length
    int id = readVarInt();
    if(id != 0){
        return false;
    }
    username = readString();
    Serial.println("[INFO] player logging in as " + username);
    return true;
}

void minecraft::writeLoginSuccess(){
    writeVarInt(18 + username.length());
    writeVarInt(0x02);
    writeUUID(1);
    writeString(username);
    Serial.println("[INFO] login success");
}

void minecraft::writeChunk(){
    writeVarInt(11+638+1026+2+2774+1); // entire packet length 
    // LENGTH 1+4+4+1+1 = 11
    writeVarInt(0x20); 
    writeInt(0); // X
    writeInt(0); // Z
    writeBoolean(1); // full chunk yes
    writeVarInt(0x01); //bitmask set to 0xFF because we're sending the whole chunk

    // NBT heightmap LENGTH 322
    for(auto i : height_map_NBT)
        S->write(i);

    // biomes LENGTH 2+1024=1026
    writeVarInt(1024); // array length 2 bytes as varint
    for(int i = 0; i < 1024; i++)
        writeVarInt(127); // 127 = void biome

    // first subchunk has one byte more because of block count varint being two bytes long
    // chunk data LENGTH 2+1+1+33+2+(8*342) = 2775
    writeVarInt(2774);

    Serial.println("writing subchunk: " + String(0));
    writeSubChunk(0);
    
    writeVarInt(0); // no block entities
    Serial.println("[INFO] chunk sent");
    //TOTAL LENGTH 11+638+1026+2775 = 4450
}

void minecraft::writePlayerPositionAndLook(double x, double y, double z, float yaw, float pitch, uint8_t flags){
    writeVarInt(35);
    writeVarInt(0x34);
    writeDouble(x);
    writeDouble(y);
    writeDouble(z);
    writeFloat(yaw);
    writeFloat(pitch);
    writeUnsignedByte(flags);
    writeVarInt(0x55);
    Serial.println("[INFO] player position and look sent");
}

void minecraft::writePlayerInfo(){
    writeVarInt(1+1+1+16+1+username.length()+1+1+1+1); //LENGTH 
    writeVarInt(0x32);
    writeVarInt(0); // action add player
    writeVarInt(1); // number of players
    writeUUID(1); // first player's uuid
    writeString(username);
    writeVarInt(0); // no properties given
    writeVarInt(0); // gamemode
    writeVarInt(100); // ping
    writeBoolean(0); // has display name
    Serial.println("[INFO] player info sent");
}

void minecraft::writeKeepAlive(){
    writeVarInt(9); //length
    writeVarInt(0x1F);
    long num = millis()/1000;
    writeLong(num);
    Serial.println("[INFO] keepalive sent: " + String(num));
}

void minecraft::writeServerDifficulty(){
    writeVarInt(3);
    writeVarInt(0x0D);
    writeUnsignedByte(0);
    writeBoolean(1);
    Serial.println("[INFO] server difficulty packet sent");
}

void minecraft::writeSpawnPlayer(){
    writeVarInt(1+1+16+8+8+8+1+1); // length
    writeVarInt(0x04);
    writeVarInt(1); // player id
    writeUUID(1); // player uuid
    writeDouble(0); // player x
    writeDouble(5); // player y
    writeDouble(0); // player z
    writeUnsignedByte(5); // player yaw
    writeUnsignedByte(0); // player pitch
    Serial.println("[INFO] spawn player packet sent");
}

void minecraft::writeJoinGame(){
    Serial.println("[INFO] sending join game packet...");
    writeVarInt(1+4+1+1+1+1+20+30478+268+20+8+1+1+1+1+1+1); // LENGTH
    writeVarInt(0x24);
    writeInt(1); // entity id
    writeBoolean(0); // is hardcore
    writeUnsignedByte(0); // gamemode
    writeByte(-1); // previous gamemode
    writeVarInt(1); // only one world
    writeString("minecraft:overworld"); // only one world
    for(auto i : dimension_codec_NBT) // NBT with world settings
        S->write(i);
    for(auto i : dimension_NBT) // NBT with valid dimensions
        S->write(i);
    writeString("minecraft:overworld"); // spawn world
    writeLong(0); // hashed seed
    writeVarInt(10); // max players
    writeVarInt(2); // view distance
    writeBoolean(0); // reduced debug info
    writeBoolean(0); // enable respawn screen
    writeBoolean(0); // is debug world
    writeBoolean(1); // is flat
    Serial.println("[INFO] join game packet sent");
}

void minecraft::writeSubChunk(uint8_t index){
    writeShort(block_count[index]); // non-air blocks
    writeUnsignedByte(5); // bits per block
    // palette
    writeVarInt(32); // in 5bits we can reference 32 different blocks
    for(auto i : blocks) // write the palette
        writeVarInt(i);
    // data
    writeVarInt(342); // we're sending 342 longs (16x16x16)/12 (one 64 bit long fits 12 5 bit blocks)

    long temp = 0;
    uint8_t count = 0;
    int count2 = 0;
    for(uint8_t i = 16 * index; i < (16 * index + 16); i++){
        for(uint8_t j = 0; j < 16; j++){
            for(uint8_t k = 0; k < 16; k++){
                temp |= ((long) blocks[chunk[i][j][k]] & 0xff) << count * 5;
                count ++;
                if(count >= 12){
                    writeLong(temp);
                    // Serial.print(temp, HEX);
                    // Serial.print(" ");
                    temp = 0;
                    count = 0;
                    count2++;
                }
            }
        }
    }
    writeLong(temp);
    count2++;
    // Serial.print(temp, HEX);
    // Serial.println();
    // Serial.println(count2);
}

int minecraft::readUnsignedShort(){
    while(S->available() < 2);
    int ret = S->read();
    return (ret << 8) | S->read();
}

float minecraft::readFloat(){
    char b[4] = {};
    while(S->available() < 4);
    for(int i=3; i>=0; i--){
        b[i] = S->read();
    }
    float f = 0;
    memcpy(&f, b, sizeof(float));
    return f;
}

double minecraft::readDouble(){
    char b[8] = {};
    while(S->available() < 8);
    for(int i=7; i>=0; i--){
        b[i] = S->read();
    }
    double d = 0;
    memcpy(&d, b, sizeof(double));
    return d;
}

long minecraft::readLong(){
    char b[8] = {};
    while(S->available() < 8);
    for(int i=0; i<8; i++){
        b[i] = S->read();
    }
    long l = ((long) b[0] << 56)
       | ((long) b[1] & 0xff) << 48
       | ((long) b[2] & 0xff) << 40
       | ((long) b[3] & 0xff) << 32
       | ((long) b[4] & 0xff) << 24
       | ((long) b[5] & 0xff) << 16
       | ((long) b[6] & 0xff) << 8
       | ((long) b[7] & 0xff);
    return l;
}

String minecraft::readString(){
    int length = readVarInt();
    Serial.println("[INFO] <- text length to read: " + String(length) + " bytes");
    String result;
    for(int i=0; i<length; i++){
        while (S->available() < 1);
        result.concat((char)S->read());
    }
    return result;
}

int minecraft::readVarInt() {
    int numRead = 0;
    int result = 0;
    byte read;
    do {
        while (S->available() < 1);
        read = S->read();
        int value = (read & 0b01111111);
        result |= (value << (7 * numRead));
        numRead++;
        if (numRead > 5) {
            Serial.println("[ERROR] VarInt too big");
        }
    } while ((read & 0b10000000) != 0);
    return result;
}

// private //
int minecraft::VarIntLength(int val) {
    if(val == 0){
        return 1;
    }
    return (int)floor(log(val) / log(128)) + 1;
}

void minecraft::writeDouble(double value){
    unsigned char * p = reinterpret_cast<unsigned char *>(&value);
    for(int i=7; i>=0; i--){
        S->write(p[i]);
    }
}

void minecraft::writeFloat(float value) {
    unsigned char * p = reinterpret_cast<unsigned char *>(&value);
    for(int i=3; i>=0; i--){
        S->write(p[i]);
    }
}

void minecraft::writeVarInt(int32_t value) {
    do {
        uint8_t temp = (uint8_t)(value & 0b01111111);
        value = lsr(value,7);
        if (value != 0) {
            temp |= 0b10000000;
        }
        S->write(temp);
    } while (value != 0);
}

void minecraft::writeVarLong(int64_t value) {
    do {
        byte temp = (byte)(value & 0b01111111);
        value = lsr(value,7);
        if (value != 0) {
            temp |= 0b10000000;
        }
        S->write(temp);
    } while (value != 0);
}

void minecraft::writeString(String str){
    int length = str.length();
    byte buf[length + 1]; 
    str.getBytes(buf, length + 1);
    writeVarInt(length);
    for(int i=0; i<length; i++){
        S->write(buf[i]);
    }
}

void minecraft::writeLong(int64_t num){
    for(int i=7; i>=0; i--){
        S->write((byte)((num >> (i*8)) & 0xff));
    }
}

void minecraft::writeUnsignedShort(uint16_t num){
    S->write((byte)((num >> 8) & 0xff));
    S->write((byte)(num & 0xff));
}

void minecraft::writeUnsignedByte(uint8_t num){
    S->write(num);
}

void minecraft::writeInt(int32_t num){
    S->write((byte)((num >> 24) & 0xff));
    S->write((byte)((num >> 16) & 0xff));
    S->write((byte)((num >> 8) & 0xff));
    S->write((byte)(num & 0xff));
}

void minecraft::writeShort(int16_t num){
    S->write((byte)((num >> 8) & 0xff));
    S->write((byte)(num & 0xff));
}

void minecraft::writeByte(int8_t num){
    S->write(num);
}

void minecraft::writeBoolean(uint8_t val){
    S->write(val);
}

void minecraft::writeUUID(int user_id){
    for(int i = 0; i < 15; i++){
        S->print(0);
    }
    S->write(user_id);
}

bool minecraft::handle_join(){
    if(!readHandShake()) return false;
    if(!readLoginStart()) return false;

    writeLoginSuccess();
    return true;
}

void minecraft::handle(){
    if(millis() - prev_keepalive > 5000){
        writeKeepAlive();
        prev_keepalive = millis();
    }

    if(S->available()){
        Serial.print(S->read(), HEX);
        Serial.print(" ");
    }
}

int32_t lsr(int32_t x, int n){
  return (int32_t)((uint32_t)x >> n);
}