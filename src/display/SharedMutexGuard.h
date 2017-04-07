template <bool shared = false>
class SharedMutexGuard
{
public:
    SharedMutexGuard(boost::shared_mutex &l) : lock(l)
    {
        if (shared)
            lock.lock_shared();
        else
            lock.lock();
        locked = true;
    }
    ~SharedMutexGuard()
    {
        Unlock();
    }
    void Unlock()
    {
        if (!locked)
            return;
        if (shared)
            lock.unlock_shared();
        else
            lock.unlock();
        locked = false;
    }
    bool locked;
    boost::shared_mutex &lock;
};
