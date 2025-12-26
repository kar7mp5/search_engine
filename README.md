# search_engine

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

## Table of Contents

- [Getting Started](#getting-started)
    - [Frontend](#frontend)
    - [Backend](#backend)
    - [Environment Variables](#environment-variables)
- [LICENSE](#license)

## Getting Started

### Frontend

- Would be updated

### Backend

- Django
- Python3.13

The backend project is based on python [uv](https://docs.astral.sh/uv/guides/install-python/)

Install uv  
Linux:
```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

Install required packages

```bash
sudo apt update
sudo apt install pkg-config
sudo apt install libcurl4-openssl-dev libgumbo-dev libxml2-dev
```

### Environment Variables

Required environment variables
```
SECRET_KEY=your-secret-key
DEBUG=True
ALLOWED_HOSTS='127.0.0.1, 127.0.0.2'
```

## LICENSE

This project is licensed under the [Apache License 2.0](./LICENSE) - see the [LICENSE](./LICENSE) file for details.
