#pragma once

enum class ClientState
{
    Accepted,
    InProgress,
    WaitingForPacketCheck,
    Done
};