#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <iosfwd>
#include <type_traits>

namespace nanolog
{
  enum class LogLevel : uint8_t { INFO, WARN, CRIT };
  
  class NanoLogLine
  {
  public:
    NanoLogLine(LogLevel level, const char* file, const char* function, uint32_t line);
    ~NanoLogLine();

    NanoLogLine(NanoLogLine &&)            = default;
    NanoLogLine& operator=(NanoLogLine &&) = default;

    void stringify(std::ostream & os);

    NanoLogLine& operator<<(char     arg);
    NanoLogLine& operator<<(int32_t  arg);
    NanoLogLine& operator<<(uint32_t arg);
    NanoLogLine& operator<<(int64_t  arg);
    NanoLogLine& operator<<(uint64_t arg);
    NanoLogLine& operator<<(double   arg);
    NanoLogLine& operator<<(const std::string& arg);

    template <size_t N>
    NanoLogLine& operator<<(const char (&arg)[N])
    {
      encode(string_literal_t(arg));
      return *this;
    }

    template <typename Arg>
    typename std::enable_if <std::is_same <Arg, const char*>::value, NanoLogLine&>::type
    operator<<(Arg const & arg)
    {
      encode(arg);
      return *this;
    }

    template <typename Arg>
    typename std::enable_if <std::is_same <Arg, char*>::value, NanoLogLine&>::type
    operator<<(Arg const & arg)
    {
      encode(arg);
      return *this;
    }

    struct string_literal_t
    {
      explicit string_literal_t(const char* s) : m_s(s) {}
      const char* m_s;
    };

  private:	
    char* buffer();

    template <typename Arg>
    void encode(Arg arg);

    template <typename Arg>
    void encode(Arg arg, uint8_t type_id);

    void encode(char* arg);
    void encode(const char* arg);
    void encode(string_literal_t arg);
    void encode_c_string(const char* arg, size_t length);
    void resize_buffer_if_needed(size_t additional_bytes);
    void stringify(std::ostream & os, char* start, const char* const end);

    size_t                   m_bytes_used;    // 已经使用的字节数
    size_t                   m_buffer_size;   // 字节大小
    std::unique_ptr<char[]>  m_heap_buffer;
    char m_stack_buffer[256 - 2 * sizeof(size_t) - sizeof(decltype(m_heap_buffer)) - 8 /* Reserved */];
  
  };  // NanoLogLine
      
  struct NanoLog
  {
    /*
    * Ideally this should have been operator+=
    * Could not get that to compile, so here we are...
    */
    bool operator==(NanoLogLine &);
  };

  void set_log_level(LogLevel level);
  
  bool is_logged(LogLevel level);

  struct NonGuaranteedLogger
  {
    NonGuaranteedLogger(uint32_t ring_buffer_size_mb_) : ring_buffer_size_mb(ring_buffer_size_mb_) {}
    uint32_t ring_buffer_size_mb;
  };

  struct GuaranteedLogger
  {
  };

  void initialize(GuaranteedLogger    gl,  const std::string& log_directory, const std::string& log_file_name, uint32_t log_file_roll_size_mb);
  void initialize(NonGuaranteedLogger ngl, const std::string& log_directory, const std::string& log_file_name, uint32_t log_file_roll_size_mb);

} // namespace nanolog

#define NANO_LOG(LEVEL) nanolog::NanoLog() == nanolog::NanoLogLine(LEVEL, __FILE__, __func__, __LINE__)
#define LOG_INFO nanolog::is_logged(nanolog::LogLevel::INFO) && NANO_LOG(nanolog::LogLevel::INFO)
#define LOG_WARN nanolog::is_logged(nanolog::LogLevel::WARN) && NANO_LOG(nanolog::LogLevel::WARN)
#define LOG_CRIT nanolog::is_logged(nanolog::LogLevel::CRIT) && NANO_LOG(nanolog::LogLevel::CRIT)