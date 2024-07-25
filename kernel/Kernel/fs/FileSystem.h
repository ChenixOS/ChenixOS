#pragma once

#include "types.h"
#include "VFS.h"
#include "Directory.h"
#include "devices/DeviceDriver.h"
#include "SuperBlock.h"

namespace VFS {

    struct Node;
    struct MountPoint;

    class FileSystem
    {
    protected:
        const char* fs_name = nullptr;
    public:
        /**
         * Reads the FileSystems SuperBlock
         **/
        virtual void GetSuperBlock(SuperBlock* sb) = 0;

        virtual void SetMountPoint(MountPoint* mp) = 0;
        virtual void PrepareUnmount() = 0;
        
        /**
         * Seeks for a suitable free Node and creates a new Node out of it
         * 寻找并创建一个Node节点
         * node --- 输出节点
         **/
        virtual void CreateNode(Node* node) = 0;

        /**
         * Destroys the given node and marks it free
         * 销毁节点
         * node --- 指定节点
         **/
        virtual void DestroyNode(Node* node) = 0;

        /**
         * Reads the Node with the given ID and writes its data into node.
         * Will only get called if a referenced node is not in the VFS node cache.
         * 读取具有指定ID的节点，并将数据反馈到node节点中
         * id --- 指定ID
         * node --- 输出节点
         **/
        virtual void ReadNode(uint64 id, Node* node) = 0; 
        /**
         * Writes the given node.
         * Will only get called if a node is ejected from the VFS node cache.
         * 写入节点数据
         * 仅在节点数据从VFS缓存中释放时调用
         * node --- 指定节点
         **/
        virtual void WriteNode(Node* node) = 0;

        /**
         * Get uncachable dir entries
         * 获取未缓存的目录节点数据
         * 
         * @param node 
         */
        virtual void UpdateDir(Node* node) = 0;

        /**
         * Reads data from the given File node.
         * node will not be locked when given to this function.
         * Blocks until at least one byte was read, or returns 0 on eof.
         **/
        virtual uint64 ReadNodeData(Node* node, uint64 pos, void* buffer, uint64 bufferSize) = 0;
        /**
         * Writes data to the given File node.
         * node will not be locked when given to this function.
         * Blocks until at least on byte was written, or returns an error code, never 0
         **/
        virtual uint64 WriteNodeData(Node* node, uint64 pos, const void* buffer, uint64 bufferSize) = 0;

        /**
         * Clears the node to an empty state.
         * Will only be called for regular file nodes.
         **/
        virtual void ClearNodeData(Node* node) = 0;

        const char* GetFileSystemName() {
            return this->fs_name;
        }
    };

    typedef FileSystem* (*FileSystemFactory)();
    typedef FileSystem* (*FileSystemFactoryDev)(BlockDeviceDriver* dev, uint64 subID);

    class FileSystemRegistry {
    public:
        static void RegisterFileSystem(const char* id, FileSystemFactory factory);
        static void RegisterFileSystem(const char* id, FileSystemFactoryDev factory);
        static void UnregisterFileSystem(const char* id);

        static FileSystem* CreateFileSystem(const char* id);
        static FileSystem* CreateFileSystem(const char* id, BlockDeviceDriver* dev, uint64 subID);
    };

}