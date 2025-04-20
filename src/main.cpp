#include "Application.h"
#include "Check.h"
#include "Exceptions.h"

int main()
{
    try
    {
        Application::Get()->Run();
    }
    catch (Jnrlib::Exceptions::JNRException const &exception)
    {
        SHOWERROR("Unexpected error: ", exception.what());
    }
    return 0;
}
