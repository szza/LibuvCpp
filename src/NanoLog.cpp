#include "NanoLog.h"
#include <string.h>
#include <time.h>

#include <string>
#include <fstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <queue>
#include <tuple>
#include <list>

namespace
{
  uint64_t timestamp_now()
  {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
  }

  void format_timestamp(std::ostream& os, uint64_t timestamp)
  {
    std::time_t time_t = timestamp / 1000000; // 以秒为单位
    auto gmtime = std::gmtime(&time_t);
    char buffer[32];
    strftime(buffer, 32, "%Y-%m-%d %T.", gmtime);
    char microseconds[7];
    sprintf(microseconds, "%06lu", timestamp % 1000000);
    os<<'['<<buffer<<microseconds<<']';
  }

  std::thread::id this_thread_id()
  {
    static thread_local const std::thread::id id = std::this_thread::get_id();
    return id;
  }

  template<typename T, typename Tuple>
  struct TupleIndex;

  template<typename T,typename ... Types>
  struct TupleIndex<T, std::tuple<T, Types...>> 
  {
    static constexpr const std::size_t value = 0;
  };

  template<typename T, typename U, typename ... Types>
  struct TupleIndex<T, std::tuple<U, Types...>> 
  {
    static constexpr const std::size_t value = 1 + TupleIndex<T, std::tuple<Types...>>::value;
  };

} // anonymous namespace

namespace nanolog
{
  using SupportedTypes = std::tuple<char, uint32_t, uint64_t, int32_t, int64_t, double, NanoLogLine::string_literal_t, char*>;

  char const* to_string(LogLevel loglevel)
  {
    switch (loglevel)
    {
    case LogLevel::INFO: return "INFO";
    case LogLevel::WARN: return "WARN";
    case LogLevel::CRIT: return "CRIT";
    }
    return "XXXX";
  }

  template<typename Arg>
  void NanoLogLine::encode(Arg arg)
  {
    *reinterpret_cast<Arg*>(buffer()) = arg;
    m_bytes_used += sizeof(Arg);
  }

  template<typename Arg>
  void NanoLogLine::encode(Arg arg, uint8_t type_id)
  {
    resize_buffer_if_needed(sizeof(Arg) + sizeof(uint8_t));
    encode<uint8_t>(type_id);
    encode<Arg>(arg);
  }

  NanoLogLine::NanoLogLine(LogLevel level, char const* file, char const* function, uint32_t line)
  : m_bytes_used(0), 
    m_buffer_size(sizeof(m_stack_buffer))
  {
    encode<uint64_t>(timestamp_now());                       // 时间
    encode<std::thread::id>(this_thread_id());               // 线程id
    encode<string_literal_t>(string_literal_t(file));        // 文件名
    encode<string_literal_t>(string_literal_t(function));    // 函数名字
    encode<uint32_t>(line);                                  // 内容行
    encode<LogLevel>(level);                                 // 等级
  }

  NanoLogLine::~NanoLogLine() = default;

  void NanoLogLine::stringify(std::ostream& os)
  {
    // 如果 heap_buffer 为空，格式化栈内存内容，否则格式化堆内存
    char* b = !m_heap_buffer ? m_stack_buffer : m_heap_buffer.get();
    char const* const end = b + m_bytes_used;
    
    // 将内存中的数据取出来，格式化到输出流中
    uint64_t          timestamp = *reinterpret_cast<uint64_t*>(b);          b += sizeof(uint64_t);
    std::thread::id   threadid  = *reinterpret_cast<std::thread::id*>(b);   b += sizeof(std::thread::id);
    string_literal_t  file      = *reinterpret_cast<string_literal_t*>(b);  b += sizeof(string_literal_t);
    string_literal_t  function  = *reinterpret_cast<string_literal_t*>(b);  b += sizeof(string_literal_t);
    uint32_t          line      = *reinterpret_cast<uint32_t*>(b);          b += sizeof(uint32_t);
    LogLevel          loglevel  = *reinterpret_cast<LogLevel*>(b);          b += sizeof(LogLevel);

    format_timestamp(os, timestamp);

    os<<'['<<to_string(loglevel)<<']'
      <<'['<<threadid<<']'
      <<'['<<file.m_s<<':'<<function.m_s<<':'<<line<<"] ";
    // 上面是头部信息
    // 下面的是真正的内容
    stringify(os, b, end);

    os<<std::endl;

    if (loglevel>= LogLevel::CRIT)
        os.flush();
  } //  stringify-end

  template<typename Arg>
  char* decode(std::ostream& os, char* b, Arg* dummy)
  {
    Arg arg = *reinterpret_cast<Arg*>(b);
    os<<arg;
    return b + sizeof(Arg);
  }

  // 特化版本
  template<>
  char* decode(std::ostream& os, char* b, NanoLogLine::string_literal_t* dummy)
  {
    NanoLogLine::string_literal_t s = *reinterpret_cast<NanoLogLine::string_literal_t*>(b);
    os<<s.m_s;
    return b + sizeof(NanoLogLine::string_literal_t);
  }

  template<>
  char* decode(std::ostream& os, char* b, char** dummy)
  {
    while (*b != '\0')
    {
        os<<*b;
        ++b;
    }
    return ++b;
  }

  void NanoLogLine::stringify(std::ostream& os, char* start, char const* const end)
  {
    if (start == end)
      return;

    int type_id = static_cast<int>(*start); start++;
    
    switch (type_id)
    {
    case 0: stringify(os, decode(os, start, static_cast<std::tuple_element<0, SupportedTypes>::type*>(nullptr)), end); return;
    case 1: stringify(os, decode(os, start, static_cast<std::tuple_element<1, SupportedTypes>::type*>(nullptr)), end); return;
    case 2: stringify(os, decode(os, start, static_cast<std::tuple_element<2, SupportedTypes>::type*>(nullptr)), end); return;
    case 3: stringify(os, decode(os, start, static_cast<std::tuple_element<3, SupportedTypes>::type*>(nullptr)), end); return;
    case 4: stringify(os, decode(os, start, static_cast<std::tuple_element<4, SupportedTypes>::type*>(nullptr)), end); return;
    case 5: stringify(os, decode(os, start, static_cast<std::tuple_element<5, SupportedTypes>::type*>(nullptr)), end); return;
    case 6: stringify(os, decode(os, start, static_cast<std::tuple_element<6, SupportedTypes>::type*>(nullptr)), end); return;
    case 7: stringify(os, decode(os, start, static_cast<std::tuple_element<7, SupportedTypes>::type*>(nullptr)), end); return;
    }
  } // stringify

  // 返回剩余的内存
  char* NanoLogLine::buffer()
  {
    return !m_heap_buffer ?&m_stack_buffer[m_bytes_used] :&(m_heap_buffer.get())[m_bytes_used];
  }
  
  void NanoLogLine::resize_buffer_if_needed(size_t additional_bytes)
  {
    size_t const required_size = m_bytes_used + additional_bytes;

    if (required_size<= m_buffer_size)
        return;

    if (!m_heap_buffer)
    {
      //  m_heap_buffer 是空的直接分配内存
      // 堆内存会释放自己之前的内存，再重新分配
      m_buffer_size = std::max(static_cast<size_t>(512), required_size);
      m_heap_buffer.reset(new char[m_buffer_size]);
      memcpy(m_heap_buffer.get(), m_stack_buffer, m_bytes_used);
      return;
    }
    else
    {   
      m_buffer_size = std::max(static_cast<size_t>(2* m_buffer_size), required_size);
      std::unique_ptr<char []> new_heap_buffer(new char[m_buffer_size]);
      memcpy(new_heap_buffer.get(), m_heap_buffer.get(), m_bytes_used);
      m_heap_buffer.swap(new_heap_buffer);
    }
  } 

  void NanoLogLine::encode(char const* arg)
  {
    if (arg != nullptr)
      encode_c_string(arg, strlen(arg));
  }

  void NanoLogLine::encode(char* arg)
  {
    if (arg != nullptr)
      encode_c_string(arg, strlen(arg));
  }

  void NanoLogLine::encode_c_string(char const* arg, size_t length)
  {
    if (length == 0)
      return;

    // type_id + 内容 + '\0'
    resize_buffer_if_needed(1 + length + 1);
    char* b = buffer();
    auto type_id = TupleIndex<char*, SupportedTypes>::value;
    *reinterpret_cast<uint8_t*>(b++) = static_cast<uint8_t>(type_id);
    memcpy(b, arg, length + 1); // 包括最后一个 '\0'
    m_bytes_used += 1 + length + 1;
  }

  void NanoLogLine::encode(string_literal_t arg)
  {
    encode<string_literal_t>(arg, TupleIndex<string_literal_t, SupportedTypes>::value);
  }

  NanoLogLine& NanoLogLine::operator<<(const std::string& arg)
  {
    encode_c_string(arg.c_str(), arg.length());
    return*this;
  }

  NanoLogLine& NanoLogLine::operator<<(int32_t arg)
  {
    encode<int32_t>(arg, TupleIndex<int32_t, SupportedTypes>::value);
    return*this;
  }

  NanoLogLine& NanoLogLine::operator<<(uint32_t arg)
  {
    encode<uint32_t>(arg, TupleIndex<uint32_t, SupportedTypes>::value);
    return*this;
  }

  NanoLogLine& NanoLogLine::operator<<(int64_t arg)
  {
    encode<int64_t>(arg, TupleIndex<int64_t, SupportedTypes>::value);
    return*this;
  }

  NanoLogLine& NanoLogLine::operator<<(uint64_t arg)
  {
    encode<uint64_t>(arg, TupleIndex<uint64_t, SupportedTypes>::value);
    return*this;
  }

  NanoLogLine& NanoLogLine::operator<<(double arg)
  {
    encode<double>(arg, TupleIndex<double, SupportedTypes>::value);
    return*this;
  }

  NanoLogLine& NanoLogLine::operator<<(char arg)
  {
    encode<char>(arg, TupleIndex<char, SupportedTypes>::value);
    return*this;
  }


  /// @biref: 自旋锁，利用 RAII 实现自旋锁的上锁与放缩
  struct SpinLock
  {
    SpinLock(std::atomic_flag& flag) : m_flag(flag)
    {
      // 一直得获得锁,即设置成功.并且之前是false
      // 即,等待其他线程放肆这个锁,这个线程再获得
      while (m_flag.test_and_set(std::memory_order_acquire));
    }

    ~SpinLock()
    {
      m_flag.clear(std::memory_order_release);
    }

  private:
    std::atomic_flag& m_flag;
  };

  struct BufferBase
  {
    virtual ~BufferBase() = default;
    virtual void push(NanoLogLine&& logline) = 0;
    virtual bool try_pop(NanoLogLine& logline) = 0;
  };
  
  // 多生产者与单一消费者
  // 循环数组
  /* Multi Producer Single Consumer Ring Buffer*/
  class RingBuffer : public BufferBase
  {
  public:
    // 每个 item 对象 对齐到 64字节边界
    // 这个对象 256 个字节 
    struct alignas(64) Item
    {
      Item() 
      : flag{ ATOMIC_FLAG_INIT }, 
        written(0), 
        logline(LogLevel::INFO, nullptr, nullptr, 0)
      { }

      std::atomic_flag  flag;       // 自旋锁, 用于保护对此对象的读写
      char              written;    // 是否写了
      char              padding[256 - sizeof(std::atomic_flag) - sizeof(char) - sizeof(NanoLogLine)];
      NanoLogLine       logline;    // 日志内容实体
    };

    RingBuffer(size_t const size) 
    : m_size(size), 
      m_ring(static_cast<Item*>(std::malloc(size* sizeof(Item)))), 
      m_write_index(0), 
      m_read_index(0)
    {
      for (size_t i = 0; i<m_size; ++i)
      {
        // placement new
        new (&m_ring[i]) Item();
      }
      static_assert(sizeof(Item) == 256, "Unexpected size != 256");
    }

    ~RingBuffer()
    {
      for (size_t i = 0; i<m_size; ++i)
      {
        m_ring[i].~Item();
      }
      std::free(m_ring);
    }

    void push(NanoLogLine&& logline) override
    {
      // std::memory_order_relaxed 只是保证自己是原子性
      uint32_t write_index = m_write_index.fetch_add(1, std::memory_order_relaxed) % m_size;
      Item& item = m_ring[write_index];
      
      // 会一直阻塞在此, 直到上锁成功
      SpinLock spinlock(item.flag);
      item.logline = std::move(logline);
      item.written = 1;
    }

    // 将内容保存至 logline 中
    bool try_pop(NanoLogLine& logline) override
    {
      Item& item = m_ring[m_read_index % m_size];
      
      SpinLock spinlock(item.flag);
      if (item.written == 1)
      {
        logline = std::move(item.logline);
        item.written = 0;
        ++m_read_index;
        return true;
      }
      return false;
    }

    // 禁止复制
    RingBuffer(RingBuffer const&) = delete;	
    RingBuffer& operator=(RingBuffer const&) = delete;

  private:
    size_t                m_size;         // buffer 中有多少个 Item 对象
    Item*                 m_ring;         // 数组
    std::atomic_uint32_t  m_write_index;  // 读指针
    uint32_t              m_read_index;   // 写指针
    char                  pad[64];
  }; // class RingBuffer

  class Buffer
  {
  public:
    // 这就是上面 ringbuffer中的item.logline
    struct Item
    {
      Item(NanoLogLine&& nanologline) 
      : logline(std::move(nanologline)) 
      { }

      char padding[256 - sizeof(NanoLogLine)];
      NanoLogLine logline;
    };


    Buffer() 
    : m_buffer(static_cast<Item*>(std::malloc(size* sizeof(Item))))
    {
      for (size_t i = 0; i<= size; ++i)
      {
        m_write_state[i].store(0, std::memory_order_relaxed);
      }
      static_assert(sizeof(Item) == 256, "Unexpected size != 256");
    }

    ~Buffer()
    {
      uint32_t write_count = m_write_state[size].load();
      for (size_t i = 0; i< write_count; ++i)
      {
        m_buffer[i].~Item();
      }
      std::free(m_buffer);
    }

    // 返回值表示这个buffer是否满了,如果满了就需要切换到下一个 buffer
    // 返回true 表示满了, false 表示没满
    bool push(NanoLogLine&& logline, const uint32_t write_index)
    {
      // placement new + 构造函数
      new (&m_buffer[write_index]) Item(std::move(logline));
      m_write_state[write_index].store(1, std::memory_order_release);
      return m_write_state[size].fetch_add(1, std::memory_order_acquire) + 1 == size;
    }

    // 逻辑上,是需要先 push 再使用 try_pop 
    // 通过 memory_order_release 与 memory_order_acquire 保证原子操作的可见性,
    // 保持线程的同步性
    bool try_pop(NanoLogLine& logline, const uint32_t read_index)
    {
      if (m_write_state[read_index].load(std::memory_order_acquire))
      {
        Item& item = m_buffer[read_index];
        logline = std::move(item.logline);
        return true;
      }
      return false;
    }

    Buffer(Buffer const&) = delete;	
    Buffer& operator=(Buffer const&) = delete;

    // 这是个编译器的静态变量, 因此可以使用为数组初始化
    static constexpr const size_t size = 32768; // 8MB. Helps reduce memory fragmentation
  private:
    Item*                 m_buffer;
    std::atomic<uint32_t> m_write_state[size + 1]; // 最后一位用来记录 m_buffer 中使用了多少个 Item 对象 
  };

  // 队列 buffer
  class QueueBuffer : public BufferBase
  {
  public:
    QueueBuffer(QueueBuffer const&) = delete;
    QueueBuffer& operator=(QueueBuffer const&) = delete;

    QueueBuffer() 
    : m_current_read_buffer{nullptr}, 
      m_write_index(0), 
      m_flag{ATOMIC_FLAG_INIT}, 
      m_read_index(0)
    {
      setup_next_write_buffer();
    }

    void push(NanoLogLine&& logline) override
    {
      uint32_t write_index = m_write_index.fetch_add(1, std::memory_order_relaxed);
      if (write_index < Buffer::size)
      {
        // 返回true,即使需要下一块内存
        if (m_current_write_buffer.load(std::memory_order_acquire)->push(std::move(logline), write_index))
        {
          setup_next_write_buffer();
        }
      }
      else
      {
        // 这里就是一直等待阻塞,直到有空余的内存
        // 再递归调用一次 push，就不会陷入死锁
        while (m_write_index.load(std::memory_order_acquire)>= Buffer::size);
        push(std::move(logline));
      }
    }

    bool try_pop(NanoLogLine& logline) override
    {
      if (m_current_read_buffer == nullptr)
        m_current_read_buffer = get_next_read_buffer();

      Buffer* read_buffer = m_current_read_buffer;

      if (read_buffer == nullptr)
        return false;

      if (read_buffer->try_pop(logline, m_read_index))
      {
        m_read_index++;
        // 当前这个buffer已经读取完毕,要从下一个 buffer 开始读取
        // 要将 
        //  1 m_current_read_buffer = nullptr
        //  2 m_read_indx =0  
        if (m_read_index == Buffer::size)
        {
          m_read_index = 0;                 // 读取指针从零开始
          m_current_read_buffer = nullptr;  // 设置为 nullptr,从下一个 buffer 开始

          SpinLock spinlock(m_flag);
          m_buffers.pop();
        }
        return true;
      }

      return false;
    }

  private:
    void setup_next_write_buffer()
    {
      std::unique_ptr<Buffer> next_write_buffer(new Buffer());
      m_current_write_buffer.store(next_write_buffer.get(), std::memory_order_release);

      SpinLock spinlock(m_flag);
      m_buffers.emplace(std::move(next_write_buffer));
      m_write_index.store(0, std::memory_order_relaxed);
    }
    
    Buffer* get_next_read_buffer()
    {
      SpinLock spinlock(m_flag);
      return m_buffers.empty() ? nullptr : m_buffers.front().get();
    }

  private:
    using BuffList     = std::queue<std::unique_ptr<Buffer>, std::list<std::unique_ptr<Buffer>>>;

    // m_buffer 是一个共享队列，因此在push/pop时候，需要用锁进行保护
    // 写时，内存不足从尾部加入新的内存，buffers 中每个读取完毕就从头部弹出

    BuffList              m_buffers;              // 从尾部写，从头部读取
    std::atomic<Buffer*>  m_current_write_buffer; // 指向 m_buffers 尾部的 buffer
    Buffer*               m_current_read_buffer;  // 指向 m_buffers 头部的 buffer
    std::atomic<uint32_t> m_write_index;          // 指示 m_current_write_buffer 写多少了  
    uint32_t              m_read_index;           // 指示 m_current_read_buffer 读取多少了
    std::atomic_flag      m_flag;                 //  用于保护 m_buffers
  }; // class QueueBuffer

  class FileWriter
  {
  public:
    FileWriter(const std::string& log_directory,  const std::string& log_file_name, uint32_t log_file_roll_size_mb)
    : m_file_number(0),
      m_bytes_written(0),
      m_log_file_roll_size_bytes(log_file_roll_size_mb* 1024* 1024), 
      m_name(log_directory + log_file_name)
    {
      roll_file();
    }

    void write(NanoLogLine& logline)
    {
      auto pos = m_os->tellp();

      logline.stringify(*m_os);
      m_bytes_written += m_os->tellp() - pos;
      // 如果大于每个文件的最大大小，那么就生成新的文件
      if (m_bytes_written> m_log_file_roll_size_bytes)
      {
        roll_file();
      }
    }

  private:
    void roll_file()
    {
      if (m_os)
      {
        m_os->flush();
        m_os->close();
      }

      m_bytes_written = 0;
      m_os.reset(new std::ofstream());

      std::string log_file_name = m_name;
      log_file_name.append(".");
      log_file_name.append(std::to_string(++m_file_number));
      log_file_name.append(".txt");
      m_os->open(log_file_name, std::ofstream::out | std::ofstream::trunc);
    }

  private:
    using OstreamPtr = std::unique_ptr<std::ofstream>;

     uint32_t         m_file_number;              // 文件编号
    std::streamoff    m_bytes_written;        
    const uint32_t    m_log_file_roll_size_bytes; // 每个日志文件大小，以 M 为单位
    const std::string m_name;                     // 日志文件名字
    OstreamPtr        m_os;                       // 输出流
  };

  class NanoLogger
  {
  public:
    NanoLogger(NonGuaranteedLogger ngl, const std::string& log_directory, const std::string& log_file_name, uint32_t log_file_roll_size_mb)
    : m_state(State::INIT), 
      m_buffer_base(new RingBuffer(std::max(1u, ngl.ring_buffer_size_mb)* 1024* 4)), 
      m_file_writer(log_directory, log_file_name, std::max(1u, log_file_roll_size_mb)), 
      m_thread([this]{this->pop();})
    {
      m_state.store(State::READY, std::memory_order_release);
    }

    NanoLogger(GuaranteedLogger gl, const std::string& log_directory, const std::string& log_file_name, uint32_t log_file_roll_size_mb)
    : m_state(State::INIT), 
      m_buffer_base(new QueueBuffer()), 
      m_file_writer(log_directory, log_file_name, std::max(1u, log_file_roll_size_mb)), 
      m_thread(&NanoLogger::pop, this)
    {
      m_state.store(State::READY, std::memory_order_release);
    }

    ~NanoLogger()
    {
      m_state.store(State::SHUTDOWN);
      m_thread.join();
    }

    void add(NanoLogLine&& logline)
    {
      m_buffer_base->push(std::move(logline));
    }
    
    void pop()
    {
      // 这里是一个线程间同步操作+sleep，等待构造函数完成
      while (m_state.load(std::memory_order_acquire) == State::INIT)
      std::this_thread::sleep_for(std::chrono::microseconds(50));
      
      NanoLogLine logline(LogLevel::INFO, nullptr, nullptr, 0);
      // 当主线程执行析构函数时，设置 State::SHUTDOWN，这个子函数才停止
      while (m_state.load() == State::READY)
      {
        // 主线程作用是 向 buffer 中写数据
        //  次线程是将数据从 buffer 中写入到文件中
        if (m_buffer_base->try_pop(logline))
          m_file_writer.write(logline);
        else
          std::this_thread::sleep_for(std::chrono::microseconds(50));
      }
      
      // Pop and log all remaining entries
      while (m_buffer_base->try_pop(logline))
      {
        m_file_writer.write(logline);
      }
    }

  private:
    enum class State {INIT, READY, SHUTDOWN}; 

    std::atomic<State>          m_state;
    std::unique_ptr<BufferBase> m_buffer_base;
    FileWriter                  m_file_writer;
    std::thread                 m_thread;
  };

  // nanologger 保存的是 Nanologger 的对象实体
  // atomic_nanologger 存储的的是对象的指针
  std::unique_ptr<NanoLogger> nanologger;
  std::atomic<NanoLogger*>    atomic_nanologger;

  bool NanoLog::operator==(NanoLogLine& logline)
  {
    atomic_nanologger.load(std::memory_order_acquire)->add(std::move(logline));
    return true;
  }

  void initialize(NonGuaranteedLogger ngl, const std::string& log_directory, const std::string& log_file_name, uint32_t log_file_roll_size_mb)
  {
    nanologger.reset(new NanoLogger(ngl, log_directory, log_file_name, log_file_roll_size_mb));
    atomic_nanologger.store(nanologger.get(), std::memory_order_seq_cst);
  }

  void initialize(GuaranteedLogger gl, const std::string& log_directory, const std::string& log_file_name, uint32_t log_file_roll_size_mb)
  {
    nanologger.reset(new NanoLogger(gl, log_directory, log_file_name, log_file_roll_size_mb));
    atomic_nanologger.store(nanologger.get(), std::memory_order_seq_cst);
  }

  std::atomic<uint32_t> loglevel = {0};

  void set_log_level(LogLevel level)
  {
    loglevel.store(static_cast<uint32_t>(level), std::memory_order_release);
  }

  bool is_logged(LogLevel level)
  {
    return static_cast<uint32_t>(level)>= loglevel.load(std::memory_order_relaxed);
  }

} // namespace nanologger