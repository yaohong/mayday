#ifndef _Buffer_h__
#define _Buffer_h__


#include "../base/Define.h"
namespace mayday
{
    namespace net
    {
        static const size_t kCheapPrepend = 8;
        static const size_t kInitialSize = 4 * 1024 * 1024;     //��ʼ��1M�Ļ���
        class Buffer
        {
            friend class TcpConnection;
        public:
            Buffer( int initialSize = kInitialSize );
        private:
            ~Buffer( );
        public:
            //���Զ�ȡ���ֽ���
            size_t readableBytes() const
            {
                return writerIndex_ - readerIndex_;
            }

            //����д����ֽ���
            size_t writableBytes() const
            {
                return buffer_.size() - writerIndex_;
            }

            //�ƶ�֮���д����ֽ���
            size_t moveAfterWritableBytes() const
            {
                return writableBytes() + (readerIndex_ - kCheapPrepend);
            }

            //��ǰ�Ķ�ȡƫ��
            size_t prependableBytes() const
            {
                return readerIndex_;
            }

            //��ǰ��ȡ����ʼ��ַ
            const char* peek() const
            {
                return begin() + readerIndex_;
            }

            //�޸Ķ�ȡƫ��
            void retrieve( size_t len )
            {
                assert( len <= readableBytes() );
                if (len < readableBytes())
                {
                    readerIndex_ += len;
                }
                else
                {
                    //��ȡƫ�Ƶ���д��ƫ�ƣ�����
                    retrieveAll();
                }
            }

            //����
            void retrieveAll()
            {
                readerIndex_ = kCheapPrepend;
                writerIndex_ = kCheapPrepend;
            }

            //��ȡָ�����ȵ��ַ���
            std::string retrieveAsString( size_t len )
            {
                assert( len <= readableBytes() );
                std::string result( peek(), len );
                retrieve( len );
                return result;
            }

            //��ǰд��ƫ�Ƶ�ַ
            char* beginWrite()
            {
                return begin() + writerIndex_;
            }

            void testWrite(const char *test)
            {
                int len = strlen( test );
                memcpy( beginWrite(), test, len);
                hasWritten( len );
            }

            int size()
            {
                return buffer_.size( );
            }

            int capacity()
            {
                return buffer_.capacity();
            }

            int readFd( int fd );

            void append( const char*  data, size_t len )
            {
                assert( len <= writableBytes() );
                std::copy( data, data + len, beginWrite() );
                hasWritten( len );
            }
        private:
            
            char* begin()
            {
                return &*buffer_.begin();
            }

            const char* begin() const
            {
                return &*buffer_.begin();
            }

            const char* beginWrite() const
            {
                return begin() + writerIndex_;
            }

            void hasWritten( size_t len )
            {
                assert( len <= writableBytes() );
                writerIndex_ += len;
            }
            void move()
            {
                if (kCheapPrepend == readerIndex_)
                {
                    return;
                }
                size_t readable = readableBytes( );
                std::copy( 
                    begin() + readerIndex_,
                    begin() + writerIndex_,
                    begin() + kCheapPrepend );
                readerIndex_ = kCheapPrepend;
                writerIndex_ = readerIndex_ + readable;
                assert( readable == readableBytes( ) );
            }
        private:
            std::vector<char> buffer_;
            size_t readerIndex_;
            size_t writerIndex_;
        };
    }
}













#endif