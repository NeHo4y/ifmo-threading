import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
val file = sc.textFile("hdfs://namenode:9000/nasa/access_log_Jul95")
val conf = new Configuration()

conf.set("fs.defaultFS", "hdfs://namenode:9000")
val fs= FileSystem.get(conf)

val os = fs.create(new Path("/results/result1.txt"))
val myLogRegex = raw"""^(\S+) (\S+) (\S+) \[([\w:/]+\s[+\-]\d{4})\] "(.+)" (\d{3}) (\S+)""".r
val rdd = file.
	filter(line => line match {
		case myLogRegex(host, client_id, user_id, datetime, request, code, size) => code(0) == '5'
		case _ => false
	}).	
	map(line => line match {
		case myLogRegex(host, client_id, user_id, datetime, request, code, size) => (request, 1)
	}).
	reduceByKey((a, b) => a + b)
rdd.
	collect().
	foreach(pair => {val str = (pair._2 + ": " + pair._1 + "\n")
                         os.write(str.getBytes)
                        })
	
