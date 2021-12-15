#include <iostream>
#include <string>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <InfluxDBFactory.h>
#include "pbPlots.hpp"
#include "supportLib.hpp"

#include <chrono>

std::chrono::system_clock::time_point createDateTime(int year, int month, int day, int hour, int minute, int second) // these are UTC values
{
    tm timeinfo1 = tm();
    timeinfo1.tm_year = year - 1900;
    timeinfo1.tm_mon = month - 1;
    timeinfo1.tm_mday = day;
    timeinfo1.tm_hour = hour;
    timeinfo1.tm_min = minute;
    timeinfo1.tm_sec = second;
    tm timeinfo = timeinfo1;
    time_t tt =  timegm(&timeinfo);
    return std::chrono::system_clock::from_time_t(tt);
}

int main(int argc,char *argv[]) {
 auto db = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=iotsensor_db");
 influxdb::Point::floatsPrecision=4;
 db->createDatabaseIfNotExists();
 for (auto i: db->query("SHOW DATABASES")) std::cout<<i.getTags()<<std::endl;
 std::ifstream file("sensor-data.csv");
 if (!file) {
  std::cout<<"Unable to open file"<<std::endl;
  return -1;
 }
 std::string dataline;
 std::vector<std::string> sensordatavector, linevector, timestampvector;
 while(getline(file,dataline)) {
  dataline.erase(remove( dataline.begin(), dataline.end(), '\"' ),dataline.end());
  sensordatavector.push_back(dataline);
 }
 std::cout<<"number of entries in sensor data "<<sensordatavector.size()<<std::endl;
 auto t_start = std::chrono::high_resolution_clock::now();
 db->batchOf(10000);
 int counter=0;
 for (std::string i : sensordatavector) {
  boost::algorithm::split(linevector, i, boost::is_any_of(","));
  boost::algorithm::split(timestampvector, linevector[0], boost::is_any_of(":- "));
  std::chrono::system_clock::time_point tss=createDateTime(std::stod(timestampvector[0]),std::stod(timestampvector[1]),std::stod(timestampvector[2]),std::stod(timestampvector[3]),std::stod(timestampvector[4]),std::stod(timestampvector[5]));
  db->write(influxdb::Point{"iotsensor"}
  .addField("power",std::stod(linevector[1]))
  .addField("temperature",std::stod(linevector[2]))
  .addField("humidity",std::stod(linevector[3]))
  .addField("light",std::stod(linevector[4]))
  .addField("co2",std::stod(linevector[5]))
  .addField("dust",std::stod(linevector[6]))
  .setTimestamp(tss));
  counter+=1;
  if (counter==10000) {
   db->flushBatch();
   counter=0;
  }
 }
 auto t_end = std::chrono::high_resolution_clock::now();
 std::cout<<"Total time (ms) for write "<<std::chrono::duration<double, std::milli>(t_end-t_start).count()<<std::endl;
 std::cout<<"database record count "<<db->query("select * from iotsensor").size()<<std::endl;

 std::vector<double> doublevector;
 std::vector<double> fieldvector[6];
 std::vector<std::string> fieldnamevector;
 sensordatavector.resize(0);linevector.resize(0);
// for (auto i: db->query("select * from iotsensor")) {
 for (auto i: db->query("select * from iotsensor where time> '2015-08-04 00:00:00' and time<'2015-08-08 01:00:00'")) {
   doublevector.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(i.getTimestamp().time_since_epoch()).count());
   boost::algorithm::split(sensordatavector, i.getFields(), boost::is_any_of(","));
   for (int j=0;j<6;j++) {
     boost::algorithm::split(linevector, sensordatavector[j], boost::is_any_of("="));
     fieldnamevector.push_back(linevector[0]);
     fieldvector[j].push_back(std::stod(linevector[1]));
   }
 }
 RGBABitmapImageReference *imageref=CreateRGBABitmapImageReference();
 std::vector<double> *pngdata;
 std::string filename;
 for (int j=0;j<6;j++) {
  DrawScatterPlot(imageref, 600, 400, &doublevector, &fieldvector[j]);
  pngdata=ConvertToPNG(imageref->image);
//  filename="plot_"+fieldnamevector[j]+std::to_string(j)+".png";
  filename="plot_partial_"+fieldnamevector[j]+std::to_string(j)+".png";
  WriteToFile(pngdata,(char*)filename.c_str());
 }
return 0;
}
