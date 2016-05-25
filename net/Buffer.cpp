#include "Buffer.h"

namespace mayday
{
    namespace net
    {
        Buffer::Buffer( int initialSize )
            : buffer_( kCheapPrepend + initialSize )
            , readerIndex_( kCheapPrepend )
            , writerIndex_( kCheapPrepend )
        {

        }

        Buffer::~Buffer( )
        {

        }

        int Buffer::readFd( int fd )
        {
            size_t writable = writableBytes( );
            if (writable == 0)
            {
                //����ԭ��ᵼ���������
                //1:һ��������BUFFERװ���£������Ӧ�÷�����������⣬��ʽ������Ӧ�ó���
                //2:��ǰʣ��ռ䲻������һ������(�п��ܷ���) ��ʽ�������ܳ��֣����ʺܵͣ�
                //3:�û��㲻��ȡ���ݣ�BUG��������⣬��ʽ������Ӧ�ó���
                if (readerIndex_ == kCheapPrepend && writerIndex_ == buffer_.size())
                {
                    //����������,��û�ж�ȡ����,���ʱ�򲻽�������
                    MDWarning("buff full");
                    assert(false);
                    return 1;
                }

                MDWarning( "buffer move start, readable:%d, writable:%d", readableBytes(), writableBytes() );
                move(); //�ƶ�λ��
                writable = writableBytes();
                MDWarning( "buffer move end, readable:%d, writable:%d", readableBytes(), writableBytes() );
                assert( writable > 0 );
            }
            int n = ::recv( fd, beginWrite( ), writable, 0 );
            if (n >= 0)
            {
                hasWritten( n );
            }

            return n;
        }
    }
}