### 技术
> 使用skiplist的简单kv store  
> 使用Reactor模式 主reactor+从reactor  
> 并发控制参考 [A Simple Optimistic Skiplist Algorithm](https://link.springer.com/chapter/10.1007/978-3-540-72951-8_11)  
### 待优化
> mit6.824,加入raft  
> 服务端错误数据处理  

### 测试
100clients, 共100万条数据插入

| 线程 | 耗时    |
| ---- | ------- |
| 1    | 58.3384 |
| 1+4  | 28.1677 |
