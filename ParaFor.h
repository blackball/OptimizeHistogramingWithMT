#include <vector>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>

// -std=c++11 needed
// pthread needed

template<typename OP, int NumThread>
struct ParaFor
{
    OP op_; // We may need to reset _op
    template <typename... Args> ParaFor(Args&&... args) : op_(std::forward<Args>(args)...) {}    
    void Join()
    {
        if (NumThread > 1)
        {
            for (int i = 0; i < NumThread; ++i)
            {
                workers_[i].currindex = i;
                workers_[i].tid = i;
                workers_[i].pf = this;
            }

            for (int i = 0; i < NumThread; ++i)
            {
                pthread_create(&threads_[i], 0, &(ParaFor<OP, NumThread>::Callback), &workers_[i]);
            }

            for (int i = 0; i < NumThread; ++i)
            {
                pthread_join(threads_[i], NULL);
            }
        }
        else
        {
            const int numitems = op_.GetNumItems();
            for (int i = 0; i < numitems; ++i)
                op_(i, 0);
        }
        op_.Post();
    }
    
private:
    ParaFor(ParaFor<OP, NumThread> const&);
    ParaFor<OP, NumThread>& operator=(ParaFor<OP, NumThread> const&);
    
    struct Worker
    {
        int currindex, tid;
        ParaFor<OP, NumThread> *pf;
    };
    
    pthread_t threads_[NumThread];
    Worker workers_[NumThread];

    static inline void* Callback(void *data)
    {
        ((Worker*)data)->pf->Work((Worker*)data);
        return NULL;
    }

    void Work(Worker *worker)
    {
        const int tid = worker->tid, numitems = op_.GetNumItems();
        int itemindex = 0;
        
        for (;;)
        {
            itemindex = __sync_fetch_and_add(&(worker->currindex), NumThread);
            if (itemindex >= numitems)
                break;
            op_(itemindex, tid);
        }
        
        itemindex = Steal();
        while (itemindex >= 0)
        {
            op_(itemindex, tid);
            itemindex = Steal();
        }
        pthread_exit(0);
    }

    int Steal()
    {
        int tid = 0, mintid = 0, nextitemindex = 0, minitemindex = INT_MAX;
        for (; tid < NumThread; ++tid)
        {
            if (minitemindex > workers_[tid].currindex)
            {
                minitemindex = workers_[tid].currindex;
                mintid = tid;
            }
        }
        
        nextitemindex = __sync_fetch_and_add(&(workers_[mintid].currindex), NumThread);
        if (nextitemindex >= op_.GetNumItems())
        {
            return -1;
        }
        return nextitemindex;
    }
};

