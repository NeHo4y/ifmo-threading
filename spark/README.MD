# Анализ NASA логов на hadoop spark

### Окружение

* docker/docker-compose

### Задача
Выполнить анализ логов на кластере Spark.

### Запуск


* Запустить кластер
```
docker-compose -f ./docker/docker-compose.yml up
```
* Download and unzip NASA logs
```
mkdir ./logs
cd ./logs
curl ftp://ita.ee.lbl.gov/traces/NASA_access_log_Jul95.gz  \
  --output log_Jul95.gz
gunzip log_Jul95.gz
curl ftp://ita.ee.lbl.gov/traces/NASA_access_log_Aug95.gz  \
  --output log_Aug95.gz
gunzip log_Aug95.gz
cd ..
```
* Загрузить логи на HDFS
```
docker cp ./logs "namenode:/logs"
docker exec namenode hadoop fs -put /logs /
```
* Загрузить scala скрипты на мастер
```
docker cp ./scala/*.scala "spark_master:/<filename>"
```
* Запустить скрипт через spark-shell и yarn
```
docker exec spark_master spark-shell -i --master yarn /<filename>
```
* Достать результаты с HDFS
```
docker exec namenode hadoop fs -get /results /
docker cp "namenode:/results" ./
```

### Результаты анализа

[Task 2](results/result1.txt)

[Task 3](results/result2.txt)

[Task 4](results/result3.txt)
