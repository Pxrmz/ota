目录结构
	/proj/application/irm/gtest/ 是用于上报模块的集成测试总目录
	
	/proj/application/irm/gtest/adm_irm_gtest.cc 是上报模块集成测试主程序文件
	/proj/application/irm/gtest/CMakeLists.txt   是上报模块集成测试主程序CMakeLists编译文件
	/proj/application/irm/gtest/config           包含上报模块集成测试可配置文件目录jsonfile和可配置文件的解析代码目录parsejson
		/proj/application/irm/gtest/config/jsonfile      上报模块集成测试可配置文件目录，目录下包含文件reportConfig.json  、testConfig.json、ucmconfig.ini
		/proj/application/irm/gtest/config/parsejson	 上报模块集成测试解析可配置文件代码目录，目录下包含如下文件：
														 adm_irm_prase_reportconfig.c、adm_irm_prase_testconfig.c、CMakeLists.txt(解析模块编译生成静态库)
	/proj/application/irm/gtest/inc              包含上报模块集成测试主程序、解析可配置文件代码的头文件，目录下包含如下文件：
												 adm_adm_prase_reportconfig.h、adm_adm_prase_testconfig.h、adm_irm_gtest.h
	/proj/application/irm/gtest/test         	 包含上报模块集成测试中可配置文件所用到的文件。
	/proj/application/irm/gtest/test/collectSysi 	collectSysi目录下有文件sysilog.zip，只要上报过程中需要收集从控的日志文件，
													那么就会以sysilog.zip文件为模板，在/tmp/目录下重新创建一个名为sysi_时间戳_序号.zip的文件（序号从0开始，序号依次叠加）；
	/proj/application/irm/gtest/test/fileList	    fileList目录下有多少个文件，外部触发文件上报就会上报多少个文件，这个由测试者控制，有则上报、无则不上报；


测试使用步骤：
	1、参考上报模块集成测试说明书根据需求改如下配置文件。
		/proj/application/irm/gtest/config/jsonfile/reportConfig.json
		/proj/application/irm/gtest/config/jsonfile/testConfig.json
	2、首先在/proj/application/irm/gtest/下新建build目录
	   然后/proj/application/irm/gtest/build下先执行" cmake .. " 然后执行" make " 
	   即可在该目录下看到生成的可执行程序irmGtest。
	   在/proj/application/irm/gtest/build目录下输入命令 " ./irmGtest -c ../config/jsonfile/ucmconfig.ini " 即可执行执行
	   产生的日志文件在/tmp/admota/ucm/log/storage/下

补充：如果需要做周期上报日志测试，需要将文件adm_irm_logFile.c：1537行睡眠600s改为30S

