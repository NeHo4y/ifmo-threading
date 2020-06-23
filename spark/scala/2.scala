import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;

val file = sc.textFile("hdfs://namenode:9000/nasa/access_log_Jul95")

val conf = new Configuration()

conf.set("fs.defaultFS", "hdfs://namenode:9000")
val fs= FileSystem.get(conf)

val os = fs.create(new Path("/results/result2.txt"))

val myLogRegex = raw"""^(\S+) (\S+) (\S+) \[([\w/]+):([\w:]+\s[+\-]\d{4})\] "(\S+) (.*)" (\d{3}) (\S+)""".r
val rdd = file.
	filter(line => line match {
		case myLogRegex(_*) => true
		case _ => false
	}).
	map(line => line match {
		case myLogRegex(host, client_id, user_id, date, time, method, request, code, size) => ((date, method, code), 1)
		case _ => (("01/Jan/1960", "UNKNOWN", -1), 1)
	}).
	reduceByKey((a, b) => a + b).
	filter(a => a._2 > 9).
	sortBy(a => a._1._1)
rdd.
	collect().
	foreach(pair => {val str = (pair._2 + ": " + pair._1 + "\n")
                         os.write(str.getBytes)
                        })

