# config
## cfg配置文件读取
    CFG文件通常具有简单的键值对结构，以节（section）和项（item）的形式组织。每个节代表一个配置类别，如数据库连接、日志设置等。每个节下包含多个项，每个项由键和值组成，用于定义具体的配置参数。例如：

    [database]
    host = localhost
    port = 3306
    username = admin
    password = secret
    [logging]
    level = INFO
    file = /var/log/myapp.log

    