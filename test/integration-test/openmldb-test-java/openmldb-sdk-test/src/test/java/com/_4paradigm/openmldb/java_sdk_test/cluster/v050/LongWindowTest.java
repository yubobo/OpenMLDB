package com._4paradigm.openmldb.java_sdk_test.cluster.v050;

import com._4paradigm.openmldb.java_sdk_test.common.OpenMLDBTest;
import com._4paradigm.openmldb.java_sdk_test.executor.ExecutorFactory;
import com._4paradigm.openmldb.test_common.model.SQLCase;
import com._4paradigm.openmldb.test_common.model.SQLCaseType;
import com._4paradigm.openmldb.test_common.provider.Yaml;
import io.qameta.allure.Story;
import lombok.extern.slf4j.Slf4j;
import org.testng.annotations.Test;

@Slf4j
public class LongWindowTest extends OpenMLDBTest {


    @Test(dataProvider = "getCase")
    @Yaml(filePaths = "function/long_window/long_window.yaml")
    @Story("Out-In")
    public void testLongWindow1(SQLCase testCase){
        ExecutorFactory.build(executor,testCase, SQLCaseType.kBatch).run();
    }

    @Test(dataProvider = "getCase")
    @Yaml(filePaths = "function/long_window/long_window.yaml")
    @Story("Out-In")
    public void testLongWindow2(SQLCase testCase){
        ExecutorFactory.build(executor,testCase, SQLCaseType.kRequest).run();
    }
}
