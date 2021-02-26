#include "minecraft.h"
#include <chunk.h>

// TODO packet serialization abstraction

// SERVERBOUND LOGIN
uint8_t minecraft::player::readHandShake(){
    readVarInt(); // length
    int id = readVarInt(); // packet id
    int protocol_version = readVarInt();
    readString(); // we don't need our name
    readUnsignedShort();
    int state = readVarInt();
    loginfo("client state = " + String(state));
    if(id != 0){
        return false;
    } else if(protocol_version != 754){
        logerr("wrong protocol version, log in with 1.16.5");
        return false;
    }

    if(state != 1 && state != 2) {
        logerr("wrong state");
        return 0;
    } else {
        return state;
    }
}

bool minecraft::player::readLoginStart(){
    readVarInt(); // length
    int id = readVarInt();
    if(id != 0){
        return false;
    }
    username = readString();
    loginfo("player logging in as " + username);
    return true;
}

uint64_t minecraft::player::readPing(){
    while(S->available() < 10);
    readVarInt(); // length
    readVarInt(); // packet id
    uint64_t payload = readLong(); // payload
    loginfo("ping received " + String((uint32_t)payload));
    return payload;
}

void minecraft::player::readRequest(){
    while(S->available() < 2);
    readVarInt();
    readVarInt();
    loginfo("request packet received");
}

// SERVERBOUND PLAY PACKETS
void minecraft::player::readChat(){
    String m = readString();
    loginfo("<" + username + "> " + m);
    mc->broadcastChatMessage(m, username);
}

void minecraft::player::readPosition(){
    x = readDouble();
    y = readDouble();
    z = readDouble();
    on_ground = readBool();
    mc->broadcastPlayerPosAndLook(x, y, z, yaw, pitch, on_ground, id);
    // loginfo("player pos " + String(x) + " " + String(y) + " " + String(z));
}

// CLIENTBOUND BROADCAST
void minecraft::broadcastChatMessage(String msg, String username){
    for(auto player : players){
        if(player.connected){
            player.loginfo("broadcasting message");
            player.writeChat(msg, username);
        }
    }
}

void minecraft::broadcastSpawnPlayer(){
    for(auto player : players){
        if(player.connected){
            for(auto p : players){
                if(p.id != player.id && p.connected){
                    player.writeSpawnPlayer(p.x, p.y, p.z, p.yaw, p.pitch, p.id);
                    player.loginfo("spawn player packet sent for p" + String(p.id));
                }
            }
        }
    }
}

void minecraft::broadcastPlayerPosAndLook(double x, double y, double z, int yaw, int pitch, bool on_ground, uint8_t id){
    for(auto player : players){
        if(player.connected && player.id != id){
            player.writeEntityTeleport(x, y, z, yaw, pitch, on_ground, id);
        }
    }
}

void minecraft::broadcastPlayerInfo(){
    // calculate data length in a horrible non-automated way for now TODO
    uint32_t num = getPlayerNum();
    uint32_t len = 3 + (21 * num);
    for(auto player : players){
        if(player.connected){
            len += player.username.length();
        }
    }
    // broadcast playerinfo
    for(auto player : players){
        if(player.connected){
            (*(player.mtx)).lock();
            player.writeVarInt(len); //LENGTH 
            player.writeVarInt(0x32);
            player.writeVarInt(0); // action add player
            player.writeVarInt(num); // number of players
            for(auto p : players){
                if(p.connected){
                    player.writeUUID(p.id); // first player's uuid
                    player.writeString(p.username);
                    player.writeVarInt(0); // no properties given
                    player.writeVarInt(0); // gamemode
                    player.writeVarInt(100); // hardcoded ping TODO
                    player.writeBoolean(0); // has display name
                }
            }
            player.loginfo("player info sent");
            (*(player.mtx)).unlock();
        }
    }
}

uint8_t minecraft::getPlayerNum(){
    uint8_t i = 0;
    for(auto player : players){
        if(player.connected) i++;
    }
    return i;
}

// CLIENTBOUND PLAYER
void minecraft::player::writeChat(String msg, String username){
    (*mtx).lock();
    String s = "{\"text\": \"<" + username + "> " + msg + "\",\"bold\": \"false\"}";
    writeVarInt(19 + s.length());
    writeVarInt(0x0E);
    writeString(s);
    writeByte(0);
    writeUUID(id);
    (*mtx).unlock();
}

void minecraft::player::writeLoginSuccess(){
    (*mtx).lock();
    writeVarInt(18 + username.length());
    writeVarInt(0x02);
    writeUUID(id);
    writeString(username);
    loginfo("login success");
    (*mtx).unlock();
}

void minecraft::player::writeChunk(){
    (*mtx).lock();
    writeVarInt(5849); // entire packet length 
    writeVarInt(0x20); 
    writeInt(0); // X
    writeInt(0); // Z
    writeBoolean(1); // full chunk yes
    writeVarInt(0x01); //bitmask set to 0xFF because we're sending the whole chunk

    S->write(height_map_NBT, sizeof(height_map_NBT) / sizeof(height_map_NBT[0]));

    uint8_t b[1024];
    memset(b, 127, 1024);
    writeVarInt(1024); // array length 2 bytes as varint
    S->write(b, 1024); // 127 = void biome
    
    //1362
    writeVarInt(4487); // first subchunk has one byte more because of block count varint being two bytes long

    Serial.println("writing subchunk: " + String(0));
    writeSubChunk(0);

    writeVarInt(0); // no block entities
    loginfo("chunk sent");
    (*mtx).unlock();
}

void minecraft::player::writePlayerPositionAndLook(double x, double y, double z, float yaw, float pitch, uint8_t flags){
    (*mtx).lock();
    writeVarInt(35);
    writeVarInt(0x34);
    writeDouble(x);
    writeDouble(y);
    writeDouble(z);
    writeFloat(yaw);
    writeFloat(pitch);
    writeUnsignedByte(flags);
    writeVarInt(0x55);
    loginfo("player position and look sent");
    (*mtx).unlock();
}

void minecraft::player::writeKeepAlive(){
    (*mtx).lock();
    writeVarInt(9); //length
    writeVarInt(0x1F);
    uint32_t num = millis()/1000;
    writeLong(num);
    loginfo("keepalive sent: " + String(num));
    (*mtx).unlock();
}

void minecraft::player::writeServerDifficulty(){
    (*mtx).lock();
    writeVarInt(3);
    writeVarInt(0x0D);
    writeUnsignedByte(0);
    writeBoolean(1);
    loginfo("server difficulty packet sent");
    (*mtx).unlock();
}

void minecraft::player::writeSpawnPlayer(double x, double y, double z, int yaw, int pitch, uint8_t id){
    (*mtx).lock();
    writeVarInt(44); // length
    writeVarInt(0x04);
    writeVarInt(id); // player id
    writeUUID(id); // player uuid
    writeDouble(x); // player x
    writeDouble(y); // player y
    writeDouble(z); // player z
    writeUnsignedByte(yaw); // player yaw
    writeUnsignedByte(pitch); // player pitch
    (*mtx).unlock();
}

void minecraft::player::writeJoinGame(){
    (*mtx).lock();
    loginfo("sending join game packet...");
    writeVarInt(1462); // LENGTH
    writeVarInt(0x24);
    writeInt(id); // entity id
    writeBoolean(0); // is hardcore
    writeUnsignedByte(0); // gamemode
    writeByte(-1); // previous gamemode
    writeVarInt(1); // only one world
    writeString("minecraft:overworld"); // only one world
    S->write(dimension_codec_NBT, sizeof(dimension_codec_NBT) / sizeof(dimension_codec_NBT[0])); // NBT with world settings
    S->write(dimension_NBT, sizeof(dimension_NBT) / sizeof(dimension_NBT[0])); // NBT with world settings
    writeString("minecraft:overworld"); // spawn world
    writeLong(0); // hashed seed
    writeVarInt(10); // max players
    writeVarInt(2); // view distance
    writeBoolean(0); // reduced debug info
    writeBoolean(0); // enable respawn screen
    writeBoolean(0); // is debug world
    writeBoolean(1); // is flat
    loginfo("join game packet sent");
    (*mtx).unlock();
}

void minecraft::player::writeResponse(){
    (*mtx).lock();
    loginfo("writing response packet...");
    writeVarInt(506);
    writeVarInt(0);
    writeString("{\"version\": {\"name\": \"1.16.5\",\"protocol\": 754},\"players\": {\"max\": 5,\"online\": 5,\"sample\": [{\"name\": \"L_S___S_S_S__S_L\",\"id\": \"00000000-0000-0000-0000-000000000000\"},{\"name\": \"L_SS__S_S_S_S__L\",\"id\": \"00000000-0000-0000-0000-000000000001\"},{\"name\": \"L_S_S_S_S_SS___L\",\"id\": \"00000000-0000-0000-0000-000000000002\"},{\"name\": \"L_S__SS_S_S_S__L\",\"id\": \"00000000-0000-0000-0000-000000000003\"},{\"name\": \"L_S___S_S_S__S_L\",\"id\": \"00000000-0000-0000-0000-000000000004\"}]},\"description\": {\"text\": \"esp32 server\"}}");
    loginfo("response packet sent");
    (*mtx).unlock();
}

void minecraft::player::writePong(uint64_t payload){
    (*mtx).lock();
    writeVarInt(9); // length
    writeVarInt(1); // packet id
    writeLong(payload); // payload
    loginfo("pong sent");
    (*mtx).unlock();
}

void minecraft::player::writeSubChunk(uint8_t index){
    writeShort(block_count[index]); // non-air blocks
    writeUnsignedByte(8); // bits per block
    writeVarInt(256); // palette length 8 bits per block
    S->write(palette, 384); // write palette
    writeVarInt(512); // we're sending 512 longs or 4096 bytes
    char * buf = (char*)chunk;
    S->write(buf, 4096);
}

void minecraft::player::writeEntityTeleport(double x, double y, double z, int yaw, int pitch, bool on_ground, uint8_t id){
    (*mtx).lock();
    writeVarInt(29); // length
    writeVarInt(0x56); // packet id
    writeVarInt(id);
    writeDouble(x);
    writeDouble(y);
    writeDouble(z);
    writeByte(yaw);
    writeByte(pitch);
    writeBoolean(on_ground);
    (*mtx).unlock();
}

// READ TYPES
uint16_t minecraft::player::readUnsignedShort(){
    while(S->available() < 2);
    int ret = S->read();
    return (ret << 8) | S->read();
}

float minecraft::player::readFloat(){
    char b[4] = {};
    while(S->available() < 4);
    for(int i=3; i>=0; i--){
        b[i] = S->read();
    }
    float f = 0;
    memcpy(&f, b, sizeof(float));
    return f;
}

double minecraft::player::readDouble(){
    char b[8] = {};
    while(S->available() < 8);
    for(int i=7; i>=0; i--){
        b[i] = S->read();
    }
    double d = 0;
    memcpy(&d, b, sizeof(double));
    return d;
}

int64_t minecraft::player::readLong(){
    char b[8] = {};
    while(S->available() < 8);
    for(int i=0; i<8; i++){
        b[i] = S->read();
    }
    uint64_t l = ((uint64_t) b[0] << 56)
       | ((uint64_t) b[1] & 0xff) << 48
       | ((uint64_t) b[2] & 0xff) << 40
       | ((uint64_t) b[3] & 0xff) << 32
       | ((uint64_t) b[4] & 0xff) << 24
       | ((uint64_t) b[5] & 0xff) << 16
       | ((uint64_t) b[6] & 0xff) << 8
       | ((uint64_t) b[7] & 0xff);
    return l;
}

String minecraft::player::readString(){
    int length = readVarInt();
    loginfo("<- text length to read: " + String(length) + " bytes");
    String result;
    for(int i=0; i<length; i++){
        while (S->available() < 1);
        result.concat((char)S->read());
    }
    return result;
}

int32_t minecraft::player::readVarInt() {
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
            logerr("VarInt too big");
        }
    } while ((read & 0b10000000) != 0);
    return result;
}

uint8_t minecraft::player::readByte(){
    return S->read();
}

bool minecraft::player::readBool(){
    return S->read();
}

// WRITE TYPES
void minecraft::player::writeDouble(double value){
    unsigned char * p = reinterpret_cast<unsigned char *>(&value);
    for(int i=7; i>=0; i--){
        S->write(p[i]);
    }
}

void minecraft::player::writeFloat(float value) {
    unsigned char * p = reinterpret_cast<unsigned char *>(&value);
    for(int i=3; i>=0; i--){
        S->write(p[i]);
    }
}

void minecraft::player::writeVarInt(int32_t value) {
    do {
        uint8_t temp = (uint8_t)(value & 0b01111111);
        value = lsr(value,7);
        if (value != 0) {
            temp |= 0b10000000;
        }
        S->write(temp);
    } while (value != 0);
}

void minecraft::player::writeVarLong(int64_t value) {
    do {
        byte temp = (byte)(value & 0b01111111);
        value = lsr(value,7);
        if (value != 0) {
            temp |= 0b10000000;
        }
        S->write(temp);
    } while (value != 0);
}

void minecraft::player::writeString(String str){
    int length = str.length();
    byte buf[length + 1]; 
    str.getBytes(buf, length + 1);
    writeVarInt(length);
    S->write(buf, length);
    /*for(int i=0; i<length; i++){
        S->write(buf[i]);
    }*/
}

void minecraft::player::writeLong(int64_t num){
    for(int i=7; i>=0; i--){
        S->write((uint8_t)((num >> (i*8)) & 0xff));
    }
}

void minecraft::player::writeUnsignedLong(uint64_t num){
    for(int i=7; i>=0; i--){
        S->write((uint8_t)((num >> (i*8)) & 0xff));
    }
}

void minecraft::player::writeUnsignedShort(uint16_t num){
    S->write((byte)((num >> 8) & 0xff));
    S->write((byte)(num & 0xff));
}

void minecraft::player::writeUnsignedByte(uint8_t num){
    S->write(num);
}

void minecraft::player::writeInt(int32_t num){
    S->write((byte)((num >> 24) & 0xff));
    S->write((byte)((num >> 16) & 0xff));
    S->write((byte)((num >> 8) & 0xff));
    S->write((byte)(num & 0xff));
}

void minecraft::player::writeShort(int16_t num){
    S->write((byte)((num >> 8) & 0xff));
    S->write((byte)(num & 0xff));
}

void minecraft::player::writeByte(int8_t num){
    S->write(num);
}

void minecraft::player::writeBoolean(uint8_t val){
    S->write(val);
}

void minecraft::player::writeUUID(int user_id){
    for(int i = 0; i < 15; i++){
        S->print(0);
    }
    S->write(user_id);
}

// HANDLERS
bool minecraft::player::join(){
    uint8_t res = readHandShake();
    if(res == 1){
        readRequest();
        delay(1000);
        writeResponse();
        uint64_t payload = readPing();
        writePong(payload);
        return false;
    } else if(res == 2){
        if(!readLoginStart()) return false;
        writeLoginSuccess();
    }
    connected = true;
    writeJoinGame();
    writePlayerPositionAndLook(0, 5, 0, 5, 0, 0x00);
    writeServerDifficulty();
    writeChunk();
    mc->broadcastPlayerInfo();
    mc->broadcastChatMessage("joined the server", username);
    mc->broadcastSpawnPlayer();
    return true;
}

void minecraft::handle(){
    for(auto player : players){
        if(player.connected){
            player.writeKeepAlive();
        }
    }
}

void minecraft::player::handle(){
    if(S->available() > 2){
        uint32_t length = S->read();
        uint32_t packetid = S->read();
        //Serial.print(S->read(), HEX);
        //Serial.print(" ");
        switch (packetid){
        case 0x03:
            readChat();
            break;
        case 0x12:
            readPosition();
            break;
        default:
            for(int i=0; i < length - VarIntLength(packetid); i++ ){
                while (S->available() < 1);
                // loginfo("packet id " + String(packetid));
                S->read();
            }
            break;
        }
    }
}

// UTILITIES
void minecraft::player::loginfo(String msg){
    Serial.println( "[INFO] p" + String(id) + " " + msg);
}

void minecraft::player::logerr(String msg){
    Serial.println( "[ERROR] p" + String(id) + " " + msg);
}

int32_t lsr(int32_t x, uint32_t n){
  return (int32_t)((uint32_t)x >> n);
}

uint32_t minecraft::player::VarIntLength(int val) {
    if(val == 0){
        return 1;
    }
    return (int)floor(log(val) / log(128)) + 1;
}