#include <iostream>
#include <InfluxDBFactory.h>
int main(int argc,char *argv[]) {
 auto db = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=temperature_db");
 influxdb::Point::floatsPrecision=4;
 db->createDatabaseIfNotExists();
 for (auto i: db->query("SHOW DATABASES")) std::cout<<i.getTags()<<std::endl;
 db->write(influxdb::Point{"temperature"}.addTag("city","DALLAS").addTag("device","companyX").addField("value",28));
 db->batchOf(10);
 for (int i=0;i<10;i++)
  db->write(influxdb::Point{"temperature"}.addTag("city","SEATTLE").addTag("device","companyY").addField("value",28+i));
 db->flushBatch();
 for (auto i: db->query("select * from temperature")) {
  std::cout<<i.getName()<<":";
  std::cout<<i.getTags()<<":";
  std::cout<<i.getFields()<<":";
  std::cout<<std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(i.getTimestamp().time_since_epoch()).count())<<std::endl;
 }
return 0;
}
