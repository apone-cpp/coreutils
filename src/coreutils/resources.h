#pragma once

#include "file.h"
#include "utils.h"

#include <memory>
#include <unordered_map>

namespace utils {

// All typed resources needs a load_data()


bool load_data(std::string& target, std::string const& file_name)
{
    target = utils::File{file_name}.readAllString();
    return true;
}

bool save_data(std::string const& source, std::string const& file_name)
{
    utils::File{file_name, utils::File::Mode::Write}.writeString(source);
    return true;
}

// template <> void save_data(utils::File& f, const std::string& data);

class Resources
{
public:
    Resources(utils::path const& load_path, utils::path const& save_path)
        : load_path_(utils::absolute(load_path)),
          save_path_(utils::absolute(save_path))
    {
        utils::create_directories(save_path_);
    }

    ~Resources() = default;
    Resources(const Resources&) = delete;

    bool done() { return true; }
    void update() {}

    template <typename T>
    void load(const std::string& fileName,
              std::function<void(std::shared_ptr<T> const& data)> onLoad)
    {
        auto fn = load_path_ / fileName;
        if(!utils::exists(fn))
            fn = save_path_ / fileName;
        auto r = std::make_shared<TypedResource<T>>(fn, onLoad);
        resources[fn] = r;
        r->load();
        setNotify(fn);
    }
    // template <typename T> void load(const std::string &fileName,
    // std::function<void(T &data)> onLoad, const T& def);
    template <typename T>
    void load(const std::string& fileName,
              std::function<void(std::shared_ptr<T> const& data)> onLoad,
              std::function<std::shared_ptr<T>()> onGenerate)
    {
        auto fn = load_path_ / fileName;
        if(!utils::exists(fn))
            fn = save_path_ / fileName;
        auto r = std::make_shared<TypedResource<T>>(fn, onLoad, onGenerate);
        resources[fn] = r;
        r->load();
        setNotify(fn);
    }

    template <typename T>
    void load(const std::string& fileName,
              std::function<void(std::shared_ptr<T> data)> onLoad,
              const T& defaultVal)
    {
        auto fn = load_path_ / fileName;
        if(!utils::exists(fn))
            fn = save_path_ / fileName;
        auto r = std::make_shared<TypedResource<T>>(
            fn, onLoad, [=]() -> std::shared_ptr<T> {
                return std::make_shared<T>(defaultVal);
            });
        resources[fn] = r;
        r->load();
        setNotify(fn);
    }

    void setNotify(const std::string& fileName) {}

private:
    class Resource
    {
    public:
        virtual ~Resource() {}
        virtual void load() {}
        virtual void generate() {}
    };

    template <typename T> class TypedResource : public Resource
    {
    public:
        TypedResource(const std::string& fileName,
                      std::function<void(std::shared_ptr<T> data)> onLoad)
            : file_name(fileName), on_load(onLoad)
        {}

        TypedResource(const std::string& fileName,
                      std::function<void(std::shared_ptr<T> data)> onLoad,
                      std::function<std::shared_ptr<T>()> onGenerate)
            : file_name(fileName), on_load(onLoad), on_generate(onGenerate)
        {}

        virtual void load() override
        {
            if (utils::exists(file_name)) {
                auto data = std::make_shared<T>();
                load_data(*(data.get()), file_name);
                on_load(data);
            } else if (on_generate) {
                auto data = on_generate();
                save_data(*data.get(), file_name);
                on_load(data);
            } else
                throw utils::io_exception(file_name);
        }

    private:
        std::string file_name;
        std::function<void(std::shared_ptr<T> data)> on_load;
        std::function<std::shared_ptr<T>()> on_generate;
    };

    utils::path save_path_;
    utils::path load_path_;

    std::unordered_map<std::string, std::shared_ptr<Resource>> resources;
    std::unordered_map<std::string, int> dirnames;
#ifdef USE_INOTIFY
    int infd;
    // int watchfd;
#endif
    int delay_counter;
};

} // namespace utils

