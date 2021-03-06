// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TERA_TABLETNODE_TABLETNODE_IMPL_H
#define TERA_TABLETNODE_TABLETNODE_IMPL_H

#include <string>

#include "common/base/scoped_ptr.h"
#include "common/thread_pool.h"

#include "tera/io/tablet_io.h"
#include "tera/master/master_client.h"
#include "tera/proto/master_rpc.pb.h"
#include "tera/proto/tabletnode.pb.h"
#include "tera/proto/tabletnode_rpc.pb.h"
#include "tera/tabletnode/rpc_compactor.h"
#include "tera/tabletnode/tabletnode_sysinfo.h"
#include "tera/utils/rpc_timer_list.h"

namespace tera {
namespace tabletnode {

class TabletManager;
class TabletNodeZkAdapter;

class TabletNodeImpl {
public:
    enum TabletNodeStatus {
        kNotInited = kTabletNodeNotInited,
        kIsIniting = kTabletNodeIsIniting,
        kIsBusy = kTabletNodeIsBusy,
        kIsReadonly = kTabletNodeIsReadonly,
        kIsRunning = kTabletNodeIsRunning
    };

    TabletNodeImpl(const TabletNodeInfo& tabletnode_info,
                   master::MasterClient* master_client,
                   TabletManager* tablet_manager = NULL);
    ~TabletNodeImpl();

    bool Init();

    bool Exit();

    bool Register();

    bool Report();

    void GarbageCollect();

    void LoadTablet(const LoadTabletRequest* request,
                    LoadTabletResponse* response,
                    google::protobuf::Closure* done);

    bool UnloadTablet(const std::string& tablet_name,
                      const std::string& start, const std::string& end,
                      StatusCode* status);

    void UnloadTablet(const UnloadTabletRequest* request,
                      UnloadTabletResponse* response,
                      google::protobuf::Closure* done);

    void CompactTablet(const CompactTabletRequest* request,
                       CompactTabletResponse* response,
                       google::protobuf::Closure* done);

    void ReadTablet(int64_t start_micros,
                    const ReadTabletRequest* request,
                    ReadTabletResponse* response,
                    google::protobuf::Closure* done,
                    ReadRpcTimer* timer = NULL);

    void WriteTablet(const WriteTabletRequest* request,
                     WriteTabletResponse* response,
                     google::protobuf::Closure* done,
                     WriteRpcTimer* timer = NULL);

    void ScanTablet(const ScanTabletRequest* request,
                    ScanTabletResponse* response,
                    google::protobuf::Closure* done);

    void GetSnapshot(const SnapshotRequest* request, SnapshotResponse* response,
                     google::protobuf::Closure* done);

    void ReleaseSnapshot(const ReleaseSnapshotRequest* request,
                         ReleaseSnapshotResponse* response,
                         google::protobuf::Closure* done);

    void Query(const QueryRequest* request, QueryResponse* response,
               google::protobuf::Closure* done);

    void SplitTablet(const SplitTabletRequest* request,
                     SplitTabletResponse* response,
                     google::protobuf::Closure* done);

    void MergeTablet(const MergeTabletRequest* request,
                     MergeTabletResponse* response,
                     google::protobuf::Closure* done);

    void EnterSafeMode();
    void LeaveSafeMode();
    void ExitService();

    void SetTabletNodeStatus(const TabletNodeStatus& status);
    TabletNodeStatus GetTabletNodeStatus();

    void SetRootTabletAddr(const std::string& root_tablet_addr);

    void SetSessionId(const std::string& session_id);
    std::string GetSessionId();

    double GetBlockCacheHitRate();

    TabletNodeSysInfo& GetSysInfo();

    void RefreshSysInfo();

    void TryReleaseMallocCache();

private:
    bool CheckInKeyRange(const KeyList& key_list,
                         const std::string& key_start,
                         const std::string& key_end);
    bool CheckInKeyRange(const KeyValueList& pair_list,
                         const std::string& key_start,
                         const std::string& key_end);
    bool CheckInKeyRange(const RowMutationList& row_list,
                         const std::string& key_start,
                         const std::string& key_end);
    bool CheckInKeyRange(const RowReaderList& reader_list,
                         const std::string& key_start,
                         const std::string& key_end);

    void UpdateMetaTableAsync(const SplitTabletRequest* request,
             SplitTabletResponse* response, google::protobuf::Closure* done,
             const std::string& path, const std::string& key_split,
             const TableSchema& schema, int64_t first_size, int64_t second_size,
             const TabletMeta& meta);
    void UpdateMetaTableCallback(const SplitTabletRequest* request,
             SplitTabletResponse* response, google::protobuf::Closure* done,
             WriteTabletRequest* request, WriteTabletResponse* response,
             bool failed, int error_code);

    void InitCacheSystem();

    void ReleaseMallocCache();
    void EnableReleaseMallocCacheTimer(int32_t expand_factor = 1);
    void DisableReleaseMallocCacheTimer();

    void GetInheritedLiveFiles(std::vector<InheritedLiveFiles>& inherited);

private:
    mutable Mutex m_status_mutex;
    TabletNodeStatus m_status;
    Mutex m_mutex;

    master::MasterClient* m_master_client;
    scoped_ptr<TabletManager> m_tablet_manager;
    scoped_ptr<TabletNodeZkAdapter> m_zk_adapter;

    uint64_t m_this_sequence_id;
    std::string m_local_addr;
    std::string m_root_tablet_addr;
    std::string m_session_id;
    int64_t m_release_cache_timer_id;

    TabletNodeSysInfo m_sysinfo;

    scoped_ptr<ThreadPool> m_thread_pool;

    RpcCompactor<MergeTabletResponse> m_merge_rpc_compactor;
    leveldb::Logger* m_ldb_logger;
    leveldb::Cache* m_ldb_block_cache;
    leveldb::TableCache* m_ldb_table_cache;
};


} // namespace tabletnode
} // namespace tera

#endif // TERA_TABLETNODE_TABLETNODE_IMPL_H
