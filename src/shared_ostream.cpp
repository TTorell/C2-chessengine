#include "shared_ostream.hpp"


namespace C2_chess
{

Shared_ostream* Shared_ostream::p_instance_{nullptr};
Shared_ostream* Shared_ostream::p_cout_instance_{nullptr};
mutex Shared_ostream::static_mutex;
} // namespace C2_chess
