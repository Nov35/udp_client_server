#include "locked_context.h"

#include "client_context_impl.h"
#include "data_chunk.h"

LockedContext::LockedContext()
    : impl_ptr_(nullptr)
{
}

LockedContext::LockedContext(ClientContextImpl *ptr)
    : impl_ptr_(ptr), lock_(ptr->GetMutex())
{
}

ClientState LockedContext::GetState()
{
    return impl_ptr_->GetState();
}

void LockedContext::SetState(const ClientState state)
{
    impl_ptr_->SetState(state);
}

void LockedContext::PrepareData(const double range)
{
    impl_ptr_->PrepareData(range);
}

DataChunk LockedContext::GetChunkOfData()
{
    return impl_ptr_->GetChunkOfData();
}

void LockedContext::NextIteration()
{
    impl_ptr_->NextIteration();
}

size_t LockedContext::CurrentChunk()
{
    return impl_ptr_->CurrentChunk();
}

asio::steady_timer &LockedContext::GetTimer()
{
    return impl_ptr_->GetTimer();
}

bool LockedContext::Empty()
{
    return impl_ptr_ == nullptr;
}

void LockedContext::Unlock()
{
    lock_.unlock();
}
