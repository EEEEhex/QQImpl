

#ifndef MMMOJO_COMMON_MMMOJO_ENVIRONMENT_CALLBACKS_H_
#define MMMOJO_COMMON_MMMOJO_ENVIRONMENT_CALLBACKS_H_

#include <stdint.h>

namespace mmmojo {
namespace common {

typedef void MMMojoReadOnPush(uint32_t request_id,
                              const void* request_info,
                              void* user_data);
typedef void MMMojoReadOnPull(uint32_t request_id,
                              const void* request_info,
                              void* user_data);
typedef void MMMojoReadOnShared(uint32_t request_id,
                                const void* request_info,
                                void* user_data);
typedef void MMMojoRemoteOnConnect(bool is_connected, void* user_data);
typedef void MMMojoRemoteOnDisConnect(void* user_data);
typedef void MMMojoRemoteOnProcessLaunched(void* user_data);
typedef void MMMojoRemoteOnProcessLaunchFailed(int error_code, void* user_data);
typedef void MMMojoRemoteOnMojoError(const void* errorbuf,
                                     int errorsize,
                                     void* user_data);

struct MMMojoEnvironmentCallbacks {
  MMMojoReadOnPush* read_on_push = nullptr;
  MMMojoReadOnPull* read_on_pull = nullptr;
  MMMojoReadOnShared* read_on_shared = nullptr;
  MMMojoRemoteOnConnect* remote_on_connect = nullptr;
  MMMojoRemoteOnDisConnect* remote_on_disconnect = nullptr;
  MMMojoRemoteOnProcessLaunched* remote_on_processlaunched = nullptr;
  MMMojoRemoteOnProcessLaunchFailed* remote_on_processlaunchfailed = nullptr;
  MMMojoRemoteOnMojoError* remote_on_mojoerror = nullptr;
};

}  // namespace common
}  // namespace mmmojo

#endif  // MMMOJO_COMMON_MMMOJO_ENVIRONMENT_CALLBACKS_H_
