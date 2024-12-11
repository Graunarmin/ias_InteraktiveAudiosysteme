#ifndef CALLBACKDATA_H
#define CALLBACKDATA_H

struct CallbackData
{
    double CallbackInterval;
    std::shared_ptr<char> ReceivedBuffer;
};

#endif // CALLBACKDATA_H
