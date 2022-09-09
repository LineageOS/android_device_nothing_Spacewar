/*
 * Copyright (C) 2023 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "UdfpsHandler.nothing_Spacewar"

#include "UdfpsHandler.h"

#include <android-base/logging.h>
#include <fcntl.h>
#include <poll.h>
#include <thread>
#include <unistd.h>

#define FOD_UI_PATH "/sys/devices/platform/soc/soc:qcom,dsi-display-primary/fod_ui"

static bool readBool(int fd) {
    char c;
    int rc;

    rc = lseek(fd, 0, SEEK_SET);
    if (rc) {
        LOG(ERROR) << "failed to seek fd, err: " << rc;
        return false;
    }

    rc = read(fd, &c, sizeof(char));
    if (rc != 1) {
        LOG(ERROR) << "failed to seek fd, err: " << rc;
        return false;
    }

    return c != '0';
}

class NothingUdfpsHander : public UdfpsHandler {
  public:
    void init(fingerprint_device_t *device) {
        mDevice = device;

    std::thread([this]() {
        int fd = open(FOD_UI_PATH, O_RDONLY);
        if (fd < 0) {
            LOG(ERROR) << "failed to open fd, err: " << fd;
            return;
        }

        struct pollfd fodUiPoll = {
                .fd = fd,
                .events = POLLERR | POLLPRI,
                .revents = 0,
        };

        while (true) {
            int rc = poll(&fodUiPoll, 1, -1);
            if (rc < 0) {
                LOG(ERROR) << "failed to poll fd, err: " << rc;
                continue;
            }

            mDevice->goodixExtCmd(mDevice, readBool(fd) ? 1 : 0, 0);
        }
    }).detach();
    }

    void onFingerDown(uint32_t /*x*/, uint32_t /*y*/, float /*minor*/, float /*major*/) {
        // nothing
    }

    void onFingerUp() {
        // nothing
    }

    void onAcquired(int32_t /*result*/, int32_t /*vendorCode*/) {
        // nothing
    }

    void cancel() {
        // nothing
    }

  private:
    fingerprint_device_t *mDevice;
};

static UdfpsHandler* create() {
    return new NothingUdfpsHander();
}

static void destroy(UdfpsHandler* handler) {
    delete handler;
}

extern "C" UdfpsHandlerFactory UDFPS_HANDLER_FACTORY = {
    .create = create,
    .destroy = destroy,
};
