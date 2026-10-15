#ifndef VC1_APP_HOST_API_H
#define VC1_APP_HOST_API_H

#include "bindings/component.h"
#include "kd/factory.h"
#include "kd/iface/icomponent_faces.h"

namespace vc1 {

struct Matrix3;
struct Matrix4;
struct RectF;

// The g5 script natives (host_api.cpp), in InitHostServices order.
KdPtr<IUnknown> getComponent(const char* name);
SquirrelObject getComponentIDs();
void quit();
void logMessage(HSQUIRRELVM v);
void logWarning(HSQUIRRELVM v);
void logError(HSQUIRRELVM v);
void logInfo(HSQUIRRELVM v);
void shutdown(const KdPtr<IAbstract>& object);
bool loadFromFile(const KdPtr<IAbstract>& object, const char* path);
bool saveToFile(const KdPtr<IAbstract>& object, const char* path);
KdPtr<IUnknown> cloneComponent(const KdPtr<IAbstract>& object);
void enableControl(const KdPtr<IAbstract>& object, bool enable);
bool isControlEnabled(const KdPtr<IAbstract>& object);
void setSlaveObject(const KdPtr<IAbstract>& object, const KdPtr<IAbstract>& slave);
bool loadFromScript(const KdPtr<IAbstract>& object, SquirrelObject table);
SquirrelObject saveToScript(const KdPtr<IAbstract>& object);
bool isIdentifiable(const KdPtr<IAbstract>& object);
const KdString& getIdentificator(const KdPtr<IAbstract>& object);
void setIdentificator(const KdPtr<IAbstract>& object, const KdString& name);
void addClassificator(const KdPtr<IAbstract>& object, int mask);
void removeClassificator(const KdPtr<IAbstract>& object, int mask);
int getClassificators(const KdPtr<IAbstract>& object);
bool checkMask(const KdPtr<IAbstract>& object, int required, int forbidden);
const Matrix3& getPosition(const KdPtr<IAbstract>& object);
void setPosition(const KdPtr<IAbstract>& object, const Matrix3& position);
void setPositionable(const KdPtr<IAbstract>& object, const KdPtr<IPositionable>& source);
bool setAnimation(const KdPtr<IAbstract>& object, const KdString& name, float speed);
bool isVisible(const KdPtr<IAbstract>& object);
void setVisible(const KdPtr<IAbstract>& object, bool visible);
bool readComponent(const KdPtr<IAbstract>& input, const KdPtr<IAbstract>& object);
int readInt(const KdPtr<IAbstract>& object);
int readShort(const KdPtr<IAbstract>& object);
int readUnsignedShort(const KdPtr<IAbstract>& object);
int readByte(const KdPtr<IAbstract>& object);
int readUnsignedByte(const KdPtr<IAbstract>& object);
bool readBool(const KdPtr<IAbstract>& object);
KdString readString(const KdPtr<IAbstract>& object);
bool writeComponent(const KdPtr<IAbstract>& output, const KdPtr<IAbstract>& object);
void writeBuffer(const KdPtr<IAbstract>& object, const char* buffer, int size);
void writeInt(const KdPtr<IAbstract>& object, int value);
void writeShort(const KdPtr<IAbstract>& object, int value);
void writeByte(const KdPtr<IAbstract>& object, int value);
void writeBool(const KdPtr<IAbstract>& object, bool value);
void writeString(const KdPtr<IAbstract>& object, const KdString& text);
void updateComponent(const KdPtr<IAbstract>& object, int layer, int dt);
void renderComponent(const KdPtr<IAbstract>& object, const KdPtr<IAbstract>& target);
KdPtr<IUnknown> beginRender(const KdPtr<IAbstract>& object, const KdPtr<IAbstract>& view);
void endRender(const KdPtr<IAbstract>& object);
KdPtr<IUnknown> getRenderTargetTexture(const KdPtr<IAbstract>& object);
RectF getBoundingRect(const KdPtr<IAbstract>& object);
int getLifeTime(const KdPtr<IAbstract>& object);
bool isExpired(const KdPtr<IAbstract>& object);
int getTime();
int getSystemTime();
int setTimer(int milliseconds, SquirrelObject callback);
void cancelTimer(int timer);
bool fileExists(const char* path);
int randNum(int range);
float randVal(float low, float high);
float angleDelta(float from, float to);
float fltsign(float value);
void TextureFilter_ColorMatrix(const KdPtr<ITexture>& texture, const Matrix4& matrix);

}

#endif
