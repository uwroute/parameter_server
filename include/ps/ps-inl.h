/**
 * @file   ps-inl.h
 * @brief  Implementation of ps.h
 */
#pragma once
#include "ps.h"
#include "kv/kv_cache.h"
#include "kv/kv_store_sparse.h"
namespace ps {

/// worker nodes
template<typename V>
KVWorker<V>::KVWorker(int id) {
  cache_ = CHECK_NOTNULL((new KVCache<Key, V>(id)));
}

template<typename V>
KVWorker<V>::~KVWorker() {
  delete cache_;
}

template<typename V>
void KVWorker<V>::Wait(int timestamp) {
  cache_->Wait(timestamp);
}


template<typename V>
int KVWorker<V>::Push(CBlob<Key> keys, CBlob<V> values, const SyncOpts& opts) {
  SBlob<Key> s_keys; s_keys.CopyFrom(keys.data, keys.size);
  SBlob<V> s_values; s_values.CopyFrom(values.data, values.size);
  return Push(s_keys, s_values, opts);
}

template<typename V>
int KVWorker<V>::Pull(CBlob<Key> keys, Blob<V> values, const SyncOpts& opts) {
  SBlob<Key> s_keys; s_keys.CopyFrom(keys.data, keys.size);
  SBlob<V> s_values(values.data, values.size, false);
  return Push(s_keys, s_values, opts);
}


template<typename V>
int KVWorker<V>::Push(const SBlob<Key>& keys, const SBlob<V>& values,
                     const SyncOpts& opts) {
  return cache_->Push(keys, values, opts);
}

template<typename V>
int KVWorker<V>::Pull(const SBlob<Key>& keys, SBlob<V>* values,
                     const SyncOpts& opts) {
  return cache_->Pull(keys, values, opts);
}


template<typename V>
void KVWorker<V>::IncrClock(int delta) {
  cache_->exector()->IncrClock(delta);
}

/// server nodes

template <typename V, typename Handle, int val_len>
void KVServer<V, Handle, val_len>::Run() {
  Customer* server = NULL;
  if (type_ == ONLINE) {
    if (val_len != kDynamicValue) {
      server = new KVStoreSparse<Key, V, Handle, val_len>(
          id_, handle_, sync_val_len_);
    }
  }
  Postoffice::instance().manager().TransferCustomer(CHECK_NOTNULL(server));
}

}  // namespace ps
