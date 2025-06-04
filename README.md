# atym-sdk
Unified C API for creating containerized applications with Atym

## Overview
The ATYM SDK provides a standardized C header file (`ocre_api.h`) that defines the API for developing embedded applications on the Atym platform. This header-only SDK enables developers to write portable code that can run across different hardware platforms.

## Usage
Include the SDK header in your application:
```c
#include "atym-sdk/ocre_api.h"

int main() {
    // Use the API functions as defined in the header
    // Implementation provided by the target platform
    
    return 0;
}
```

## Integration
This SDK is included as a submodule in the [getting-started](https://github.com/atym-io/getting-started) repository. To get started with Atym development:

```bash
git clone --recursive https://github.com/atym-io/getting-started.git
```

For standalone projects, you can add this SDK as a submodule:

```bash
git submodule add https://github.com/atym-io/atym-sdk.git
```

## License
MIT