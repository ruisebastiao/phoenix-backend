#include "catch.hpp"

#include "corecontroller.hpp"

#include <QDebug>

SCENARIO("The core controller can handle loading a game and core")
{
  GIVEN("") {
    struct MockMemory {
      void writeVideoFrame(const char*, uint, uint, uint) {}
      bool isOpened() { return true; }
      bool open(int) { return true; }
    };

    struct MockCore {
      MockCore()
      {
        retro_init = []() {};
        retro_set_audio_sample = [](retro_audio_sample_t) {};
        retro_set_audio_sample_batch = [](retro_audio_sample_batch_t) {};
        retro_set_controller_port_device = [](unsigned, unsigned) {};
        retro_set_environment = [](retro_environment_t) {};
        retro_set_input_poll = [](retro_input_poll_t) {};
        retro_set_input_state = [](retro_input_state_t) {};
        retro_set_video_refresh = [](retro_video_refresh_t) {};
        retro_get_system_info = [](struct retro_system_info * info) {
          info->library_name = "fake library";
          info->library_version = "fake version";
          info->valid_extensions = "fake exts";
          info->need_fullpath = true;
          info->block_extract = true;
        };
        retro_load_game = [](const struct retro_game_info*) -> bool {
          return true;
        };
        retro_get_system_av_info = [](struct retro_system_av_info*) {};
      }
      void load(QString) {}
      void (*retro_set_audio_sample)(retro_audio_sample_t);
      void (*retro_set_audio_sample_batch)(retro_audio_sample_batch_t);
      void (*retro_set_controller_port_device)(unsigned, unsigned);
      void (*retro_set_environment)(retro_environment_t);
      void (*retro_set_input_poll)(retro_input_poll_t);
      void (*retro_set_input_state)(retro_input_state_t);
      void (*retro_set_video_refresh)(retro_video_refresh_t);

      void (*retro_init)(void);

      void (*retro_get_system_info)(struct retro_system_info*);
      bool (*retro_load_game)(const struct retro_game_info*);
      void (*retro_get_system_av_info)(struct retro_system_av_info*);

    };

    static const char* gameFilePath = "game/file/path.rom";
    static const char* gameBufferData = "game/buffer/data";
    static const int gameBufferSize = 256;

    struct MockGame {
      void open(QString) {}
      void close() {}
      const char* constData() const { return gameBufferData; }
      int size() const { return gameBufferSize; }
      const char* filePath() const { return gameFilePath; }
      void copyToRam() {}
    };

    using Subject = CoreController_T<MockMemory, MockCore, MockGame>;
    Subject subject;
    WHEN("init() is called with a core and game path") {
      REQUIRE_NOTHROW(subject.init("/core/path", "/game/path"));
    }

    WHEN("openGame() is called with 'need_fullpath = true'") {
      retro_system_info systemInfo = {};
      systemInfo.need_fullpath = true;

      THEN("the game info path is filled in") {
        retro_game_info gameInfo = subject.openGame(systemInfo);
        REQUIRE(gameInfo.path == gameFilePath);
        REQUIRE(gameInfo.data == nullptr);
        REQUIRE(gameInfo.size == 0);
        REQUIRE(QString(gameInfo.meta) == "");
      }
    }

    WHEN("openGame() is called with 'need_fullpath = false'") {
      retro_system_info systemInfo = {};
      systemInfo.need_fullpath = false;

      THEN("the game info path is filled in") {
        retro_game_info gameInfo = subject.openGame(systemInfo);
        REQUIRE(gameInfo.path == nullptr);
        REQUIRE(gameInfo.data == gameBufferData);
        REQUIRE(gameInfo.size == gameBufferSize);
        REQUIRE(QString(gameInfo.meta) == "");
      }
    }
  }
}
