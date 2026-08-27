#include <ArduinoJson.h>
#include <vector>

class ISettingNode
{
public:
    virtual ~ISettingNode() = default;
    virtual void loadFromJson(JsonObject& obj) = 0;
    virtual void saveToJson(JsonObject& obj) const = 0;
};

class SettingGroup : public ISettingNode
{
protected:
    const char* key;
    std::vector<ISettingNode*> children;

public:
    SettingGroup(SettingGroup* parent, const char* keyName) : key(keyName)
    {
        if (parent != nullptr)
        {
            parent->addChild(this);
        }
    }

    void loadFromJson(JsonObject& obj) override
    {
        if (obj[key].is<JsonObject>())
        {
            JsonObject childObj = obj[key].as<JsonObject>();
            for (auto* child : children) child->loadFromJson(childObj);
        }
    }

    void saveToJson(JsonObject& obj) const override
    {
        JsonObject childObj = obj[key].to<JsonObject>();
        for (auto* child : children) child->saveToJson(childObj);
    }

    void addChild(ISettingNode* child)
    {
        children.push_back(child);
    }
};

template <typename T>
class Setting : public ISettingNode
{
private:
    const char* key;
    T value, defaultValue;
    bool (*validator)(T);

public:
    Setting(SettingGroup* parent, const char* keyName, T defVal, bool (*validFunc)(T) = nullptr)
        : key(keyName), value(defVal), defaultValue(defVal), validator(validFunc)
    {
        if (parent != nullptr)
        {
            parent->addChild(this);
        }
    }

    T get() const { return value; }
    bool set(T newValue)
    {
        if (validator && !validator(newValue)) return false;
        value = newValue;
        return true;
    }

    void loadFromJson(JsonObject& obj) override
    {
        if (obj[key].is<T>())
        {
            if (!set(obj[key].as<T>())) value = defaultValue;
        }
    }

    void saveToJson(JsonObject& obj) const override
    {
        obj[key] = value;
    }
};

template <size_t MaxLen>
class StringSetting : public ISettingNode {
private:
    const char* key;
    char value[MaxLen];
    char defaultValue[MaxLen];

public:
    StringSetting(SettingGroup* parent, const char* keyName, const char* defVal) 
        : key(keyName) {
        
        // Copie sécurisée (évite les buffer overflows)
        strlcpy(value, defVal, MaxLen);
        strlcpy(defaultValue, defVal, MaxLen);
        
        if (parent != nullptr)
        {
            parent->addChild(this);
        }
    }

    const char* get() const { return value; }

    void loadFromJson(JsonObject& obj) override {
        if (obj[key].is<const char*>()) {
            // ArduinoJson ou cJSON sait copier proprement vers un char[]
            strlcpy(value, obj[key].as<const char*>(), MaxLen);
        }
    }

    void saveToJson(JsonObject& obj) const override {
        obj[key] = value;
    }
};

#define BEGIN_GROUP(ClassName, instanceName) \
    struct ClassName : public SettingGroup { \
        ClassName(SettingGroup* parent) : SettingGroup(parent, #instanceName) {}

#define END_GROUP(instanceName) \
    } instanceName{this};

#define PARAM(type, name, defaultValue) \
    Setting<type> name{this, #name, defaultValue};

#define PARAM_VALIDATED(type, name, defaultValue, validatorFunc) \
    Setting<type> name{this, #name, defaultValue, validatorFunc};

#define PARAM_STR(name, maxLength, defaultValue) \
    StringSetting<maxLength> name{this, #name, defaultValue};
