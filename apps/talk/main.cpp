#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#define __STDC_FORMAT_MACROS 1

#include <inttypes.h>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include "agora_sdk_i.h"

using namespace std;
using namespace agora;
using namespace agora::pstn;

inline int64_t now_ms() {
  timeval now;
  gettimeofday(&now, NULL);
  return now.tv_sec * 1000ll + now.tv_usec / 1000;
}

#if !defined(LOG)
#include <cstdio>
#define LOG(level, fmt, ...) printf("%" PRId64 ": " fmt "\n", now_ms(), ## __VA_ARGS__)
#endif

#ifdef __GNUC__

//#if __GNUC_PREREQ(4, 6)
#include <atomic>
typedef std::atomic<bool> atomic_bool_t;
//#else
//typedef volatile bool atomic_bool_t;
//#endif

//#if __GNUC_PREREQ(4, 7)
#define OVERRIDE override
//#else
//#define OVERRIDE
//#endif

#endif

atomic_bool_t g_quit_flag;

enum ChannelState {kInit, kJoinFailed, kQuit};

class AgoraChannel : private IAgoraSdkEventHandler {
 public:
  AgoraChannel(const string &vendorKey, const string &channelName, uint32_t uid=0,
      bool audioMixed=true, int32_t sampleRates=8000);

  virtual ~AgoraChannel();

  // Spawn a thread to run the loop of fetching data.
  // return immediately
  void Start();

  // Stop the worker thread.
  // return until the worker thread quits the channel.
  void Stop();

  // Query the channel state.
  // bool GetChannelState() const;
 private:
  virtual void onJoinSuccess(const char *cname, unsigned uid, const char *msg) OVERRIDE;
  virtual void onError(int rescode, const char *msg) OVERRIDE;
  virtual void onVoiceData(uint32_t uid, const char *pbuffer, uint32_t length) OVERRIDE;
  virtual void onSessionCreate(const char *sessionId) OVERRIDE;
 private:
  int Run();
 private:
  string vendor_key_;
  string channel_name_;
  uint32_t uid_;
  bool audio_mixed_;
  int32_t sample_rates_;

  FILE *pcm_file_;
  atomic_bool_t joined_flag_;
  atomic_bool_t quit_flag_;

  std::thread worker_;
};

AgoraChannel::AgoraChannel(const string &vendorKey, const string &channelName,
    uint32_t uid, bool audioMixed, int32_t sampleRates)
    :vendor_key_(vendorKey), channel_name_(channelName), uid_(uid),
    audio_mixed_(audioMixed), sample_rates_(sampleRates) {
  string file_name = "./" + channelName + ".pcm";
  pcm_file_ = fopen(file_name.c_str(), "wb");
  if (!pcm_file_) {
    LOG(FATAL, "Failed to open pcm file to write: %s",
        strerror(errno));
    abort();
  }

  quit_flag_ = false;
  joined_flag_ = false;
}

AgoraChannel::~AgoraChannel() {
  fclose(pcm_file_);
}

void AgoraChannel::onJoinSuccess(const char *cname, unsigned uid, const char *msg) {
  joined_flag_ = true;
  LOG(INFO, "User %u joined the channel %s: %s", uid, cname, msg);
}

void AgoraChannel::onError(int rescode, const char *msg) {
  LOG(ERROR, "Error: %d, %s", rescode, msg);
  quit_flag_ = true;
}

void AgoraChannel::onVoiceData(unsigned int uid, const char *buf,
    uint32_t length) {
  (void)uid;
  (void)buf;
  (void)length;
  // LOG(INFO, "voice data received: %d, from %d", int(length), int(uid));
  fwrite(buf, 1, length, pcm_file_);
}

void AgoraChannel::onSessionCreate(const char *sessionId) {
  LOG(INFO, "Session created: %s", sessionId);
}

void AgoraChannel::Start() {
  worker_ = std::thread(&AgoraChannel::Run, this);
}

int AgoraChannel::Run() {

  IAgoraSdk *sdk = IAgoraSdk::createInstance(this);
  if (!sdk) {
    LOG(ERROR, "Failed to create the instance of AgoraSdk while joining the "
        "channel %s", channel_name_.c_str());
    return -1;
  }

  LOG(INFO, "joining channel: %s, key: %s\n", channel_name_.c_str(),
      vendor_key_.c_str());

  sdk->joinChannel(NULL, vendor_key_.c_str(), channel_name_.c_str(), uid_,
      0, 0, audio_mixed_, sample_rates_);

  // Invoke |onTimer| every 10 ms
  std::chrono::milliseconds duration(10);
 
  while (true) {
    // Fetch voice data from the SDK
    sdk->onTimer();
    if (quit_flag_ || g_quit_flag)
      break;

    std::this_thread::sleep_for(duration);
  }

  LOG(INFO, "leaving......\n");
  sdk->leave();

  IAgoraSdk::destroyInstance(sdk);

  return 0;
}

void AgoraChannel::Stop() {
  worker_.join();
}

void interrupt_handler(int sig_no) {
  (void)sig_no;
  LOG(INFO, "signal");
  g_quit_flag = true;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
      LOG(ERROR, "Not enough arguments.");
      return -1;
  }

  char* channelName = argv[1];
  //char* hostId = argv[2];

  g_quit_flag = false;
  signal(SIGINT, interrupt_handler);

  // SIGPIPE Must be captured.
  signal(SIGPIPE, SIG_IGN);

  std::vector<AgoraChannel *> channels;
  const char *cnames[] = {
    "Recording_test_1",
  };

  cnames[0] = channelName;

  static const char vendor_key[] = "7289f274bc9b4468bf887f38075e8604";
  for (unsigned i = 0; i < sizeof(cnames) / sizeof(cnames[0]); ++i) {
    AgoraChannel *p = new (std::nothrow)AgoraChannel(vendor_key,
        cnames[i], 0, true, 8000);
    p->Start();
    channels.push_back(p);
    LOG(INFO, "%d channel name %s\n", i, cnames[i]);
  }
  
  for (unsigned i = 0; i < sizeof(cnames) / sizeof(cnames[0]); ++i) {
    AgoraChannel *p = channels[i];
    p->Stop();
    delete p;
  }

  LOG(INFO, "All threads joined!\n");
  return 0;
}


