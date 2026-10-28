
********dimtool工具********
主控侧目录结构：
--dim_lua_3th： 第三方库，主要是c与json互调的api
--dim_lvm： 入参构造
--dis_lvm： 响应构造(不常用，测试链路太短)

工具使用调试框架：
                                             dim_lvm(入参构造)
                                                |
                                               dim
                                  ----------------------------
                                  |             |            |
                                 dis_lvm       dis           
                                            ---------
                                            |       |
                                            arpc    oem
                                            |
                                 --------------------------
                                 |                        |
                                sysi                     oem
                          -----------------------
                          |          |          |
                         sysi_lvm  vdi/ucl等   oem
                                  -------
                                  |     |
                                 can   doip
                                  --------
                                  |     |
                            udsServer  oem





