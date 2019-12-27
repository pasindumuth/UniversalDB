#ifndef UNI_ASSERT_UNIVERSALEXCEPTION_H
#define UNI_ASSERT_UNIVERSALEXCEPTION_H

#include <exception>
#include <string>

#include <common/common.h>

namespace uni {
namespace assert {

class UniversalException: std::exception {
 public:
  explicit UniversalException(const std::string& message);

  ~UniversalException() override = default;

  const char* what() const noexcept override;

 private:
  const std::string _message;
};

} // namespace assert
} // namespace uni

#endif // UNI_ASSERT_UNIVERSALEXCEPTION_H
