#include "locked_context.h"

#include "client_context_impl.h"
#include "data_chunk.h"

LockedContext::LockedContext()
    : context_ptr_(nullptr)
{
}

LockedContext::LockedContext(ClientContextImpl *ptr)
    : context_ptr_(ptr), lock_(ptr->GetMutex())
{
}

ClientState LockedContext::GetState()
{
    return context_ptr_->GetState();
}

void LockedContext::SetState(const ClientState state)
{
    context_ptr_->SetState(state);
}

void LockedContext::PrepareData(const double range)
{
    context_ptr_->PrepareData(range);
}

DataChunk LockedContext::GetChunkOfData()
{
    return context_ptr_->GetChunkOfData();
}

void LockedContext::NextIteration()
{
    context_ptr_->NextIteration();
}

size_t LockedContext::GetCurrentIteration()
{
    return context_ptr_->GetCurrentIteration();
}

bool LockedContext::Empty()
{
    return context_ptr_ == nullptr;
}
