/*-------------------------------------------------------------------------
 * Copyright (C) 2019, 4paradigm
 * planner_test.cc
 *
 * Author: chenjing
 * Date: 2019/10/24
 *--------------------------------------------------------------------------
 **/

#include "plan/planner.h"
#include <utility>
#include <vector>
#include "gtest/gtest.h"
#include "parser/parser.h"
namespace fesql {
namespace plan {

using fesql::node::NodeManager;
using fesql::node::PlanNode;
using fesql::node::SQLNode;
using fesql::node::SQLNodeList;
// TODO(chenjing): add ut: 检查SQL的语法树节点预期 2019.10.23
class PlannerTest : public ::testing::TestWithParam<std::string> {
 public:
    PlannerTest() {
        manager_ = new NodeManager();
        parser_ = new parser::FeSQLParser();
    }

    ~PlannerTest() {
        delete parser_;
        delete manager_;
    }

 protected:
    parser::FeSQLParser *parser_;
    NodeManager *manager_;
};

INSTANTIATE_TEST_CASE_P(
    SqlExprPlanner, PlannerTest,
    testing::Values(
        "SELECT COL1 FROM t1;", "SELECT COL1 as c1 FROM t1;",
        "SELECT COL1 c1 FROM t1;", "SELECT t1.COL1 FROM t1;",
        "SELECT t1.COL1 as c1 FROM t1;", "SELECT t1.COL1 c1 FROM t1;",
        "SELECT t1.COL1 c1 FROM t1 limit 10;", "SELECT * FROM t1;",
        "SELECT COUNT(*) FROM t1;", "SELECT COUNT(COL1) FROM t1;",
        "SELECT TRIM(COL1) FROM t1;", "SELECT trim(COL1) as trim_col1 FROM t1;",
        "SELECT MIN(COL1) FROM t1;", "SELECT min(COL1) FROM t1;",
        "SELECT MAX(COL1) FROM t1;", "SELECT max(COL1) as max_col1 FROM t1;",
        "SELECT SUM(COL1) FROM t1;", "SELECT sum(COL1) as sum_col1 FROM t1;",
        "SELECT COL1, COL2, `TS`, AVG(COL3) OVER w, SUM(COL3) OVER w FROM t1 \n"
        "WINDOW w AS (PARTITION BY COL2\n"
        "              ORDER BY `TS` ROWS BETWEEN UNBOUNDED PRECEDING AND "
        "UNBOUNDED FOLLOWING);",
        "SELECT COL1, trim(COL2), `TS`, AVG(AMT) OVER w, SUM(AMT) OVER w FROM "
        "t1 \n"
        "WINDOW w AS (PARTITION BY COL2\n"
        "              ORDER BY `TS` ROWS BETWEEN 3 PRECEDING AND 3 "
        "FOLLOWING);",
        "SELECT COL1, SUM(COL3) OVER w as w_amt_sum FROM t \n"
        "WINDOW w AS (PARTITION BY COL2\n"
        "              ORDER BY `TS` ROWS BETWEEN 3 PRECEDING AND 3 "
        "FOLLOWING);",
        "SELECT COL1 + COL2 as col12 FROM t1;",
        "SELECT COL1 - COL2 as col12 FROM t1;",
        "SELECT COL1 * COL2 as col12 FROM t1;",
        "SELECT COL1 / COL2 as col12 FROM t1;",
        "SELECT COL1 % COL2 as col12 FROM t1;",
        "SELECT COL1 = COL2 as col12 FROM t1;",
        "SELECT COL1 == COL2 as col12 FROM t1;",
        "SELECT COL1 < COL2 as col12 FROM t1;",
        "SELECT COL1 > COL2 as col12 FROM t1;",
        "SELECT COL1 <= COL2 as col12 FROM t1;",
        "SELECT COL1 != COL2 as col12 FROM t1;",
        "SELECT COL1 >= COL2 as col12 FROM t1;",
        "SELECT COL1 >= COL2 && COL1 != COL2 as col12 FROM t1;",
        "SELECT COL1 >= COL2 and COL1 != COL2 as col12 FROM t1;",
        "SELECT COL1 >= COL2 || COL1 != COL2 as col12 FROM t1;",
        "SELECT COL1 >= COL2 or COL1 != COL2 as col12 FROM t1;",
        "SELECT !(COL1 >= COL2 or COL1 != COL2) as col12 FROM t1;",

        "SELECT sum(col1) OVER w1 as w1_col1_sum FROM t1 "
        "WINDOW w1 AS (PARTITION BY col15 ORDER BY `TS` RANGE BETWEEN 3 "
        "PRECEDING AND CURRENT ROW) limit 10;",
        "SELECT COUNT(*) FROM t1;"));

INSTANTIATE_TEST_CASE_P(
    SqlWherePlan, PlannerTest,
    testing::Values(
        "SELECT COL1 FROM t1 where COL1+COL2;",
        "SELECT COL1 FROM t1 where COL1;",
        "SELECT COL1 FROM t1 where COL1 > 10 and COL2 = 20 or COL1 =0;",
        "SELECT COL1 FROM t1 where COL1 > 10 and COL2 = 20;",
        "SELECT COL1 FROM t1 where COL1 > 10;"));
INSTANTIATE_TEST_CASE_P(
    SqlLikePlan, PlannerTest,
    testing::Values("SELECT COL1 FROM t1 where COL like \"%abc\";",
                    "SELECT COL1 FROM t1 where COL1 like '%123';",
                    "SELECT COL1 FROM t1 where COL not like \"%abc\";",
                    "SELECT COL1 FROM t1 where COL1 not like '%123';",
                    "SELECT COL1 FROM t1 where COL1 not like 10;",
                    "SELECT COL1 FROM t1 where COL1 like 10;"));
INSTANTIATE_TEST_CASE_P(
    SqlInPlan, PlannerTest,
    testing::Values(
        "SELECT COL1 FROM t1 where COL in (1, 2, 3, 4, 5);",
        "SELECT COL1 FROM t1 where COL1 in (\"abc\", \"xyz\", \"test\");",
        "SELECT COL1 FROM t1 where COL1 not in (1,2,3,4,5);"));

INSTANTIATE_TEST_CASE_P(
    SqlGroupPlan, PlannerTest,
    testing::Values(
        "SELECT distinct sum(COL1) as col1sum, * FROM t1 where col2 > 10 group "
        "by COL1, "
        "COL2 having col1sum > 0 order by COL1+COL2 limit 10;",
        "SELECT sum(COL1) as col1sum, * FROM t1 group by COL1, COL2;",
        "SELECT COL1 FROM t1 group by COL1+COL2;",
        "SELECT COL1 FROM t1 group by COL1;",
        "SELECT COL1 FROM t1 group by COL1 > 10 and COL2 = 20 or COL1 =0;",
        "SELECT COL1 FROM t1 group by COL1, COL2;",
        "SELECT COL1 FROM t1 group by COL1;"));

INSTANTIATE_TEST_CASE_P(
    SqlHavingPlan, PlannerTest,
    testing::Values(
        "SELECT COL1 FROM t1 having COL1+COL2;",
        "SELECT COL1 FROM t1 having COL1;",
        "SELECT COL1 FROM t1 HAVING COL1 > 10 and COL2 = 20 or COL1 =0;",
        "SELECT COL1 FROM t1 HAVING COL1 > 10 and COL2 = 20;",
        "SELECT COL1 FROM t1 HAVING COL1 > 10;"));

INSTANTIATE_TEST_CASE_P(
    SqlOrderPlan, PlannerTest,
    testing::Values("SELECT COL1 FROM t1 order by COL1 + COL2 - COL3;",
                    "SELECT COL1 FROM t1 order by COL1, COL2, COL3;",
                    "SELECT COL1 FROM t1 order by COL1, COL2;",
                    "SELECT COL1 FROM t1 order by COL1;"));

INSTANTIATE_TEST_CASE_P(
    SqlWhereGroupHavingOrderPlan, PlannerTest,
    testing::Values(
        "SELECT sum(COL1) as col1sum, * FROM t1 where col2 > 10 group by COL1, "
        "COL2 having col1sum > 0 order by COL1+COL2 limit 10;",
        "SELECT sum(COL1) as col1sum, * FROM t1 where col2 > 10 group by COL1, "
        "COL2 having col1sum > 0 order by COL1 limit 10;",
        "SELECT sum(COL1) as col1sum, * FROM t1 where col2 > 10 group by COL1, "
        "COL2 having col1sum > 0 limit 10;",
        "SELECT sum(COL1) as col1sum, * FROM t1 where col2 > 10 group by COL1, "
        "COL2 having col1sum > 0;",
        "SELECT sum(COL1) as col1sum, * FROM t1 group by COL1, COL2 having "
        "sum(COL1) > 0;",
        "SELECT sum(COL1) as col1sum, * FROM t1 group by COL1, COL2 having "
        "col1sum > 0;"));

INSTANTIATE_TEST_CASE_P(
    SqlJoinPlan, PlannerTest,
    testing::Values("SELECT * FROM t1 full join t2 on t1.col1 = t2.col2;",
                    "SELECT * FROM t1 left join t2 on t1.col1 = t2.col2;",
                    "SELECT * FROM t1 right join t2 on t1.col1 = t2.col2;",
                    "SELECT * FROM t1 inner join t2 on t1.col1 = t2.col2;"));

INSTANTIATE_TEST_CASE_P(
    SqlUnionPlan, PlannerTest,
    testing::Values(
        "SELECT * FROM t1 UNION SELECT * FROM t2;",
        "SELECT * FROM t1 UNION DISTINCT SELECT * FROM t2;",
        "SELECT * FROM t1 UNION ALL SELECT * FROM t2;",
        "SELECT * FROM t1 UNION ALL SELECT * FROM t2 UNION SELECT * FROM t3;",
        "SELECT * FROM t1 left join t2 on t1.col1 = t2.col2 UNION ALL SELECT * "
        "FROM t3 UNION SELECT * FROM t4;",
        "SELECT sum(COL1) as col1sum, * FROM t1 where col2 > 10 group by COL1, "
        "COL2 having col1sum > 0 order by COL1+COL2 limit 10 UNION ALL "
        "SELECT sum(COL1) as col1sum, * FROM t1 group by COL1, COL2 having "
        "sum(COL1) > 0;",
        "SELECT * FROM t1 inner join t2 on t1.col1 = t2.col2 UNION "
        "SELECT * FROM t3 inner join t4 on t3.col1 = t4.col2 UNION "
        "SELECT * FROM t5 inner join t6 on t5.col1 = t6.col2;"));
INSTANTIATE_TEST_CASE_P(
    SqlDistinctPlan, PlannerTest,
    testing::Values(
        "SELECT distinct COL1 FROM t1 HAVING COL1 > 10 and COL2 = 20;",
        "SELECT DISTINCT sum(COL1) as col1sum, * FROM t1 group by COL1,COL2;",
        "SELECT DISTINCT sum(col1) OVER w1 as w1_col1_sum FROM t1 "
        "WINDOW w1 AS (PARTITION BY col15 ORDER BY `TS` RANGE BETWEEN 3 "
        "PRECEDING AND CURRENT ROW) limit 10;",
        "SELECT DISTINCT COUNT(*) FROM t1;",
        "SELECT distinct COL1 FROM t1 where COL1+COL2;",
        "SELECT DISTINCT COL1 FROM t1 where COL1 > 10;"));

INSTANTIATE_TEST_CASE_P(
    SqlSubQueryPlan, PlannerTest,
    testing::Values(
        "SELECT * FROM t1 WHERE COL1 > (select avg(COL1) from t1) limit 10;",
        "select * from (select * from t1 where col1>0);",
        "SELECT LastName,FirstName, Title, Salary FROM Employees AS T1 "
        "WHERE Salary >=(SELECT Avg(Salary) "
        "FROM Employees WHERE T1.Title = Employees.Title) Order by Title;",
        "select * from \n"
        "    (select * from stu where grade = 7) s\n"
        "left join \n"
        "    (select * from sco where subject = \"math\") t\n"
        "on s.id = t.stu_id\n"
        "union\n"
        "select distinct * from \n"
        "    (select distinct * from stu where grade = 7) s\n"
        "right join \n"
        "    (select distinct * from sco where subject = \"math\") t\n"
        "on s.id = t.stu_id;",
        "SELECT * FROM t5 inner join t6 on t5.col1 = t6.col2;",
        "select * from \n"
        "    (select * from stu where grade = 7) s\n"
        "left join \n"
        "    (select * from sco where subject = \"math\") t\n"
        "on s.id = t.stu_id\n"
        "union\n"
        "select * from \n"
        "    (select * from stu where grade = 7) s\n"
        "right join \n"
        "    (select * from sco where subject = \"math\") t\n"
        "on s.id = t.stu_id;",
        "SELECT * FROM t5 inner join t6 on t5.col1 = t6.col2;"));
TEST_P(PlannerTest, PlannerSucessTest) {
    std::string sqlstr = GetParam();
    std::cout << sqlstr << std::endl;

    NodePointVector trees;
    base::Status status;
    int ret = parser_->parse(sqlstr.c_str(), trees, manager_, status);

    if (0 != status.code) {
        std::cout << status.msg << std::endl;
    }
    ASSERT_EQ(0, ret);
    //    ASSERT_EQ(1, trees.size());
    //    std::cout << *(trees.front()) << std::endl;
    Planner *planner_ptr = new SimplePlanner(manager_);
    node::PlanNodeList plan_trees;
    ASSERT_EQ(0, planner_ptr->CreatePlanTree(trees, plan_trees, status));
    ASSERT_EQ(1u, plan_trees.size());
    std::cout << *(plan_trees.front()) << std::endl;
}

TEST_F(PlannerTest, SimplePlannerCreatePlanTest) {
    node::NodePointVector list;
    base::Status status;
    int ret = parser_->parse(
        "SELECT t1.COL1 c1,  trim(COL3) as trimCol3, COL2 FROM t1 limit 10;",
        list, manager_, status);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(1u, list.size());

    Planner *planner_ptr = new SimplePlanner(manager_);
    node::PlanNodeList plan_trees;
    ASSERT_EQ(0, planner_ptr->CreatePlanTree(list, plan_trees, status));
    ASSERT_EQ(1u, plan_trees.size());
    PlanNode *plan_ptr = plan_trees.front();
    std::cout << *(plan_ptr) << std::endl;
    //     validate select plan
    ASSERT_EQ(node::kPlanTypeQuery, plan_ptr->GetType());
    plan_ptr = plan_ptr->GetChildren()[0];

    ASSERT_EQ(node::kPlanTypeLimit, plan_ptr->GetType());
    node::LimitPlanNode *limit_ptr = (node::LimitPlanNode *)plan_ptr;
    // validate project list based on current row
    ASSERT_EQ(10, limit_ptr->GetLimitCnt());
    ASSERT_EQ(node::kPlanTypeProject,
              limit_ptr->GetChildren().at(0)->GetType());

    node::ProjectPlanNode *project_plan_node =
        (node::ProjectPlanNode *)limit_ptr->GetChildren().at(0);
    ASSERT_EQ(1u, project_plan_node->project_list_vec_.size());

    node::ProjectListNode *project_list = dynamic_cast<node::ProjectListNode *>(
        project_plan_node->project_list_vec_[0]);

    ASSERT_EQ(0u,
              dynamic_cast<node::ProjectNode *>(project_list->GetProjects()[0])
                  ->GetPos());
    ASSERT_EQ(1u,
              dynamic_cast<node::ProjectNode *>(project_list->GetProjects()[1])
                  ->GetPos());
    ASSERT_EQ(2u,
              dynamic_cast<node::ProjectNode *>(project_list->GetProjects()[2])
                  ->GetPos());

    plan_ptr = project_plan_node->GetChildren()[0];
    // validate limit 10
    ASSERT_EQ(node::kPlanTypeTable, plan_ptr->GetType());
    node::TablePlanNode *relation_node =
        reinterpret_cast<node::TablePlanNode *>(plan_ptr);
    ASSERT_EQ("t1", relation_node->table_);
    delete planner_ptr;
}

TEST_F(PlannerTest, SelectPlanWithWindowProjectTest) {
    node::NodePointVector list;
    node::PlanNodeList trees;
    base::Status status;
    int ret = parser_->parse(
        "SELECT COL1, SUM(AMT) OVER w1 as w_amt_sum FROM t \n"
        "WINDOW w1 AS (PARTITION BY COL2\n"
        "              ORDER BY `TS` ROWS BETWEEN 3 PRECEDING AND 3 "
        "FOLLOWING) limit 10;",
        list, manager_, status);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(1u, list.size());

    std::cout << *(list[0]) << std::endl;
    Planner *planner_ptr = new SimplePlanner(manager_);
    ASSERT_EQ(0, planner_ptr->CreatePlanTree(list, trees, status));
    ASSERT_EQ(1u, trees.size());
    PlanNode *plan_ptr = trees[0];
    ASSERT_TRUE(NULL != plan_ptr);

    std::cout << *plan_ptr << std::endl;
    // validate select plan
    ASSERT_EQ(node::kPlanTypeQuery, plan_ptr->GetType());
    plan_ptr = plan_ptr->GetChildren()[0];
    // validate limit node
    ASSERT_EQ(node::kPlanTypeLimit, plan_ptr->GetType());
    node::LimitPlanNode *limit_ptr = (node::LimitPlanNode *)plan_ptr;
    // validate project list based on current row
    ASSERT_EQ(10, limit_ptr->GetLimitCnt());
    ASSERT_EQ(node::kPlanTypeProject,
              limit_ptr->GetChildren().at(0)->GetType());

    node::ProjectPlanNode *project_plan_node =
        (node::ProjectPlanNode *)limit_ptr->GetChildren().at(0);
    plan_ptr = project_plan_node;
    ASSERT_EQ(1u, project_plan_node->project_list_vec_.size());

    // validate projection 0
    node::ProjectListNode *project_list = dynamic_cast<node::ProjectListNode *>(
        project_plan_node->project_list_vec_[0]);

    ASSERT_EQ(2u, project_list->GetProjects().size());

    ASSERT_TRUE(nullptr != project_list->GetW());
    ASSERT_EQ(-3, project_list->GetW()->GetStartOffset());
    ASSERT_EQ(3, project_list->GetW()->GetEndOffset());

    ASSERT_EQ(std::vector<std::string>({"COL2"}),
              project_list->GetW()->GetKeys());
    ASSERT_TRUE(project_list->IsWindowAgg());

    plan_ptr = plan_ptr->GetChildren()[0];
    ASSERT_EQ(node::kPlanTypeTable, plan_ptr->GetType());
    node::TablePlanNode *relation_node =
        reinterpret_cast<node::TablePlanNode *>(plan_ptr);
    ASSERT_EQ("t", relation_node->table_);
    delete planner_ptr;
}

TEST_F(PlannerTest, SelectPlanWithMultiWindowProjectTest) {
    node::NodePointVector list;
    node::PlanNodeList trees;
    base::Status status;
    const std::string sql =
        "SELECT sum(col1) OVER w1 as w1_col1_sum, sum(col1) OVER w2 as "
        "w2_col1_sum FROM t1 "
        "WINDOW "
        "w1 AS (PARTITION BY col2 ORDER BY `TS` RANGE BETWEEN 1d PRECEDING AND "
        "1s PRECEDING), "
        "w2 AS (PARTITION BY col3 ORDER BY `TS` RANGE BETWEEN 2d PRECEDING AND "
        "1s PRECEDING) "
        "limit 10;";
    int ret = parser_->parse(sql, list, manager_, status);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(1u, list.size());
    std::cout << sql << std::endl;
    //        std::cout << *(list[0]) << std::endl;
    Planner *planner_ptr = new SimplePlanner(manager_);
    ASSERT_EQ(0, planner_ptr->CreatePlanTree(list, trees, status));
    ASSERT_EQ(1u, trees.size());
    PlanNode *plan_ptr = trees[0];
    ASSERT_TRUE(NULL != plan_ptr);

    std::cout << *plan_ptr << std::endl;
    // validate select plan
    ASSERT_EQ(node::kPlanTypeQuery, plan_ptr->GetType());
    plan_ptr = plan_ptr->GetChildren()[0];
    // validate limit node
    ASSERT_EQ(node::kPlanTypeLimit, plan_ptr->GetType());
    node::LimitPlanNode *limit_ptr = (node::LimitPlanNode *)plan_ptr;
    // validate project list based on current row
    ASSERT_EQ(10, limit_ptr->GetLimitCnt());
    ASSERT_EQ(node::kPlanTypeProject,
              limit_ptr->GetChildren().at(0)->GetType());

    node::ProjectPlanNode *project_plan_node =
        (node::ProjectPlanNode *)limit_ptr->GetChildren().at(0);
    plan_ptr = project_plan_node;
    ASSERT_EQ(2u, project_plan_node->project_list_vec_.size());

    // validate projection 1: window agg over w1
    node::ProjectListNode *project_list = dynamic_cast<node::ProjectListNode *>(
        project_plan_node->project_list_vec_[0]);

    ASSERT_EQ(1u, project_list->GetProjects().size());
    ASSERT_TRUE(nullptr != project_list->GetW());

    ASSERT_TRUE(project_list->IsWindowAgg());

    ASSERT_EQ(-1 * 86400000, project_list->GetW()->GetStartOffset());
    ASSERT_EQ(-1000, project_list->GetW()->GetEndOffset());
    ASSERT_EQ(std::vector<std::string>({"col2"}),
              project_list->GetW()->GetKeys());

    // validate projection 1: window agg over w2
    project_list = dynamic_cast<node::ProjectListNode *>(
        project_plan_node->project_list_vec_[1]);
    ASSERT_EQ(1u, project_list->GetProjects().size());
    ASSERT_TRUE(nullptr != project_list->GetW());
    ASSERT_EQ(-2 * 86400000, project_list->GetW()->GetStartOffset());
    ASSERT_EQ(-1000, project_list->GetW()->GetEndOffset());

    ASSERT_EQ(std::vector<std::string>({"col3"}),
              project_list->GetW()->GetKeys());
    ASSERT_TRUE(project_list->IsWindowAgg());

    plan_ptr = plan_ptr->GetChildren()[0];
    ASSERT_EQ(node::kPlanTypeTable, plan_ptr->GetType());
    node::TablePlanNode *relation_node =
        reinterpret_cast<node::TablePlanNode *>(plan_ptr);
    ASSERT_EQ("t1", relation_node->table_);
    delete planner_ptr;
}

TEST_F(PlannerTest, MultiProjectListPlanPostTest) {
    node::NodePointVector list;
    node::PlanNodeList trees;
    base::Status status;
    const std::string sql =
        "%%fun\n"
        "def test_col_at(col:list<float>, pos:i32):float\n"
        "\treturn col[pos]\n"
        "end\n"
        "%%sql\n"
        "SELECT sum(col1) OVER w1 as w1_col1_sum, "
        "sum(col3) OVER w2 as w2_col3_sum, "
        "sum(col4) OVER w2 as w2_col4_sum, "
        "col1, "
        "sum(col3) OVER w1 as w1_col3_sum, "
        "col2, "
        "sum(col1) OVER w2 as w2_col1_sum, "
        "test_col_at(col1, 0) OVER w2 as w2_col1_at_0, "
        "test_col_at(col1, 1) OVER w2 as w2_col1_at_1 "
        "FROM t1 "
        "WINDOW "
        "w1 AS (PARTITION BY col2 ORDER BY `TS` RANGE BETWEEN 1d PRECEDING AND "
        "1s PRECEDING), "
        "w2 AS (PARTITION BY col3 ORDER BY `TS` RANGE BETWEEN 2d PRECEDING AND "
        "1s PRECEDING) "
        "limit 10;";
    int ret = parser_->parse(sql, list, manager_, status);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(2u, list.size());

    std::cout << *(list[0]) << std::endl;
    std::cout << *(list[1]) << std::endl;
    Planner *planner_ptr = new SimplePlanner(manager_);
    ASSERT_EQ(0, planner_ptr->CreatePlanTree(list, trees, status));
    ASSERT_EQ(2u, trees.size());

    PlanNode *plan_ptr = trees[1];
    ASSERT_TRUE(NULL != plan_ptr);

    std::cout << *plan_ptr << std::endl;
    // validate select plan
    ASSERT_EQ(node::kPlanTypeQuery, plan_ptr->GetType());
    plan_ptr = plan_ptr->GetChildren()[0];

    // validate limit node
    ASSERT_EQ(node::kPlanTypeLimit, plan_ptr->GetType());
    node::LimitPlanNode *limit_ptr = (node::LimitPlanNode *)plan_ptr;

    // validate project list based on current row
    ASSERT_EQ(10, limit_ptr->GetLimitCnt());
    ASSERT_EQ(node::kPlanTypeProject,
              limit_ptr->GetChildren().at(0)->GetType());

    node::ProjectPlanNode *project_plan_node =
        (node::ProjectPlanNode *)limit_ptr->GetChildren().at(0);
    plan_ptr = project_plan_node;
    ASSERT_EQ(2u, project_plan_node->project_list_vec_.size());

    const std::vector<std::pair<uint32_t, uint32_t>> pos_mapping =
        project_plan_node->pos_mapping_;
    ASSERT_EQ(9u, pos_mapping.size());
    ASSERT_EQ(std::make_pair(0u, 0u), pos_mapping[0]);
    ASSERT_EQ(std::make_pair(1u, 0u), pos_mapping[1]);
    ASSERT_EQ(std::make_pair(1u, 1u), pos_mapping[2]);
    ASSERT_EQ(std::make_pair(0u, 1u), pos_mapping[3]);
    ASSERT_EQ(std::make_pair(0u, 2u), pos_mapping[4]);
    ASSERT_EQ(std::make_pair(0u, 3u), pos_mapping[5]);
    ASSERT_EQ(std::make_pair(1u, 2u), pos_mapping[6]);
    ASSERT_EQ(std::make_pair(1u, 3u), pos_mapping[7]);
    ASSERT_EQ(std::make_pair(1u, 4u), pos_mapping[8]);

    // validate projection 0: window agg over w1
    {
        node::ProjectListNode *project_list =
            dynamic_cast<node::ProjectListNode *>(
                project_plan_node->project_list_vec_.at(0));

        ASSERT_EQ(4u, project_list->GetProjects().size());
        ASSERT_FALSE(nullptr == project_list->GetW());
        ASSERT_EQ(-1 * 86400000, project_list->GetW()->GetStartOffset());
        ASSERT_EQ(-1000, project_list->GetW()->GetEndOffset());

        // validate w1_col1_sum pos 0
        {
            node::ProjectNode *project = dynamic_cast<node::ProjectNode *>(
                project_list->GetProjects()[0]);
            ASSERT_EQ(0u, project->GetPos());
        }
        // validate col1 pos 3
        {
            node::ProjectNode *project = dynamic_cast<node::ProjectNode *>(
                project_list->GetProjects()[1]);
            ASSERT_EQ(3u, project->GetPos());
        }

        // validate w1_col3_sum pos 0
        {
            node::ProjectNode *project = dynamic_cast<node::ProjectNode *>(
                project_list->GetProjects()[2]);
            ASSERT_EQ(4u, project->GetPos());
        }

        // validate col2 pos 5
        {
            node::ProjectNode *project = dynamic_cast<node::ProjectNode *>(
                project_list->GetProjects()[3]);
            ASSERT_EQ(5u, project->GetPos());
        }
    }
    {
        // validate projection 1: window agg over w2
        node::ProjectListNode *project_list =
            dynamic_cast<node::ProjectListNode *>(
                project_plan_node->project_list_vec_.at(1));

        ASSERT_EQ(5u, project_list->GetProjects().size());
        ASSERT_TRUE(nullptr != project_list->GetW());
        ASSERT_EQ(-2 * 86400000, project_list->GetW()->GetStartOffset());
        ASSERT_EQ(-1000, project_list->GetW()->GetEndOffset());
        ASSERT_EQ(std::vector<std::string>({"col3"}),
                  project_list->GetW()->GetKeys());
        ASSERT_TRUE(project_list->IsWindowAgg());

        // validate w2_col3_sum pos 1
        {
            node::ProjectNode *project = dynamic_cast<node::ProjectNode *>(
                project_list->GetProjects()[0]);
            ASSERT_EQ(1u, project->GetPos());
        }
        // validate w2_col4_sum pos 2
        {
            node::ProjectNode *project = dynamic_cast<node::ProjectNode *>(
                project_list->GetProjects()[1]);
            ASSERT_EQ(2u, project->GetPos());
        }
        // validate w2_col1_sum pos 6
        {
            node::ProjectNode *project = dynamic_cast<node::ProjectNode *>(
                project_list->GetProjects()[2]);
            ASSERT_EQ(6u, project->GetPos());
        }

        // validate w2_col1_at_0 pos 7
        {
            node::ProjectNode *project = dynamic_cast<node::ProjectNode *>(
                project_list->GetProjects()[3]);
            ASSERT_EQ(7u, project->GetPos());
        }
        // validate w2_col1_at_1 pos 8
        {
            node::ProjectNode *project = dynamic_cast<node::ProjectNode *>(
                project_list->GetProjects()[4]);
            ASSERT_EQ(8u, project->GetPos());
        }
    }

    plan_ptr = plan_ptr->GetChildren()[0];
    ASSERT_EQ(node::kPlanTypeTable, plan_ptr->GetType());
    node::TablePlanNode *relation_node =
        reinterpret_cast<node::TablePlanNode *>(plan_ptr);
    ASSERT_EQ("t1", relation_node->table_);
    delete planner_ptr;
}

TEST_F(PlannerTest, CreateStmtPlanTest) {
    const std::string sql_str =
        "create table IF NOT EXISTS test(\n"
        "    column1 int NOT NULL,\n"
        "    column2 timestamp NOT NULL,\n"
        "    column3 int NOT NULL,\n"
        "    column4 string NOT NULL,\n"
        "    column5 int NOT NULL,\n"
        "    index(key=(column4, column3), ts=column2, ttl=60d)\n"
        ");";

    node::NodePointVector list;
    node::PlanNodeList trees;
    base::Status status;
    int ret = parser_->parse(sql_str, list, manager_, status);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(1u, list.size());
    std::cout << *(list[0]) << std::endl;

    Planner *planner_ptr = new SimplePlanner(manager_);
    ASSERT_EQ(0, planner_ptr->CreatePlanTree(list, trees, status));
    ASSERT_EQ(1u, trees.size());
    PlanNode *plan_ptr = trees[0];
    ASSERT_TRUE(NULL != plan_ptr);

    std::cout << *plan_ptr << std::endl;

    // validate create plan
    ASSERT_EQ(node::kPlanTypeCreate, plan_ptr->GetType());
    node::CreatePlanNode *createStmt = (node::CreatePlanNode *)plan_ptr;

    type::TableDef table_def;
    ASSERT_TRUE(TransformTableDef(createStmt->GetTableName(),
                                  createStmt->GetColumnDescList(), &table_def,
                                  status));

    type::TableDef *table = &table_def;
    ASSERT_EQ("test", table->name());
    ASSERT_EQ(5, table->columns_size());
    ASSERT_EQ("column1", table->columns(0).name());
    ASSERT_EQ("column2", table->columns(1).name());
    ASSERT_EQ("column3", table->columns(2).name());
    ASSERT_EQ("column4", table->columns(3).name());
    ASSERT_EQ("column5", table->columns(4).name());
    ASSERT_EQ(type::Type::kInt32, table->columns(0).type());
    ASSERT_EQ(type::Type::kTimestamp, table->columns(1).type());
    ASSERT_EQ(type::Type::kInt32, table->columns(2).type());
    ASSERT_EQ(type::Type::kVarchar, table->columns(3).type());
    ASSERT_EQ(type::Type::kInt32, table->columns(4).type());
    ASSERT_EQ(1, table->indexes_size());
    ASSERT_EQ(60 * 86400000UL, table->indexes(0).ttl(0));
    ASSERT_EQ(2, table->indexes(0).first_keys_size());
    ASSERT_EQ("column4", table->indexes(0).first_keys(0));
    ASSERT_EQ("column3", table->indexes(0).first_keys(1));
    ASSERT_EQ("column2", table->indexes(0).second_key());
    // TODO(chenjing): version and version count test
    //    ASSERT_EQ("column5", index_node->GetVersion());
    //    ASSERT_EQ(3, index_node->GetVersionCount());
}

TEST_F(PlannerTest, CmdStmtPlanTest) {
    const std::string sql_str = "show databases;";

    node::NodePointVector list;
    node::PlanNodeList trees;
    base::Status status;
    int ret = parser_->parse(sql_str, list, manager_, status);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(1u, list.size());
    std::cout << *(list[0]) << std::endl;

    Planner *planner_ptr = new SimplePlanner(manager_);
    ASSERT_EQ(0, planner_ptr->CreatePlanTree(list, trees, status));
    ASSERT_EQ(1u, trees.size());
    PlanNode *plan_ptr = trees[0];
    ASSERT_TRUE(NULL != plan_ptr);

    std::cout << *plan_ptr << std::endl;

    // validate create plan
    ASSERT_EQ(node::kPlanTypeCmd, plan_ptr->GetType());
    node::CmdPlanNode *cmd_plan = (node::CmdPlanNode *)plan_ptr;
    ASSERT_EQ(node::kCmdShowDatabases, cmd_plan->GetCmdType());
}

TEST_F(PlannerTest, FunDefPlanTest) {
    const std::string sql_str =
        "%%fun\ndef test(a:i32,b:i32):i32\n    c=a+b\n    d=c+1\n    return "
        "d\nend";

    node::NodePointVector list;
    node::PlanNodeList trees;
    base::Status status;
    int ret = parser_->parse(sql_str, list, manager_, status);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(1u, list.size());
    std::cout << *(list[0]) << std::endl;

    Planner *planner_ptr = new SimplePlanner(manager_);
    ASSERT_EQ(0, planner_ptr->CreatePlanTree(list, trees, status));
    std::cout << status.msg << std::endl;
    ASSERT_EQ(1u, trees.size());
    PlanNode *plan_ptr = trees[0];
    ASSERT_TRUE(NULL != plan_ptr);

    std::cout << *plan_ptr << std::endl;

    // validate create plan
    ASSERT_EQ(node::kPlanTypeFuncDef, plan_ptr->GetType());
    node::FuncDefPlanNode *plan =
        dynamic_cast<node::FuncDefPlanNode *>(plan_ptr);
    ASSERT_TRUE(nullptr != plan->fn_def_);
    ASSERT_TRUE(nullptr != plan->fn_def_->header_);
    ASSERT_TRUE(nullptr != plan->fn_def_->block_);
}

TEST_F(PlannerTest, FunDefAndSelectPlanTest) {
    const std::string sql_str =
        "%%fun\ndef test(a:i32,b:i32):i32\n    c=a+b\n    d=c+1\n    return "
        "d\nend\n%%sql\nselect col1, test(col1, col2) from t1 limit 1;";

    node::NodePointVector list;
    node::PlanNodeList trees;
    base::Status status;
    int ret = parser_->parse(sql_str, list, manager_, status);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(2u, list.size());
    std::cout << *(list[0]) << std::endl;
    std::cout << *(list[1]) << std::endl;

    Planner *planner_ptr = new SimplePlanner(manager_);
    ASSERT_EQ(0, planner_ptr->CreatePlanTree(list, trees, status));
    std::cout << status.msg << std::endl;
    ASSERT_EQ(2u, trees.size());
    PlanNode *plan_ptr = trees[0];
    ASSERT_TRUE(NULL != plan_ptr);
    std::cout << *plan_ptr << std::endl;

    // validate fundef plan
    ASSERT_EQ(node::kPlanTypeFuncDef, plan_ptr->GetType());
    node::FuncDefPlanNode *plan =
        dynamic_cast<node::FuncDefPlanNode *>(plan_ptr);

    ASSERT_TRUE(nullptr != plan->fn_def_);
    ASSERT_TRUE(nullptr != plan->fn_def_->header_);
    ASSERT_TRUE(nullptr != plan->fn_def_->block_);

    // validate select plan
    plan_ptr = trees[1];
    ASSERT_TRUE(NULL != plan_ptr);
    std::cout << *plan_ptr << std::endl;
    // validate select plan

    ASSERT_EQ(node::kPlanTypeQuery, plan_ptr->GetType());
    plan_ptr = plan_ptr->GetChildren()[0];
    ASSERT_EQ(node::kPlanTypeLimit, plan_ptr->GetType());
    node::LimitPlanNode *limit_plan =
        dynamic_cast<node::LimitPlanNode *>(plan_ptr);
    ASSERT_EQ(1, limit_plan->GetLimitCnt());
}

TEST_F(PlannerTest, FunDefIfElsePlanTest) {
    const std::string sql_str =
        "%%fun\n"
        "def test(a:i32,b:i32):i32\n"
        "    c=a+b\n"
        "\td=c+1\n"
        "\tif a<b\n"
        "\t\treturn c\n"
        "\telif c > d\n"
        "\t\treturn d\n"
        "\telif d > 1\n"
        "\t\treturn c+d\n"
        "\telse \n"
        "\t\treturn d\n"
        "end\n"
        "%%sql\n"
        "select col1, test(col1, col2) from t1 limit 1;";
    std::cout << sql_str;

    node::NodePointVector list;
    node::PlanNodeList trees;
    base::Status status;
    int ret = parser_->parse(sql_str, list, manager_, status);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(2u, list.size());
    std::cout << *(list[0]) << std::endl;
    std::cout << *(list[1]) << std::endl;

    Planner *planner_ptr = new SimplePlanner(manager_);
    ASSERT_EQ(0, planner_ptr->CreatePlanTree(list, trees, status));
    std::cout << status.msg << std::endl;
    ASSERT_EQ(2u, trees.size());
    PlanNode *plan_ptr = trees[0];
    ASSERT_TRUE(NULL != plan_ptr);
    std::cout << *plan_ptr << std::endl;

    // validate fundef plan
    ASSERT_EQ(node::kPlanTypeFuncDef, plan_ptr->GetType());
    node::FuncDefPlanNode *plan =
        dynamic_cast<node::FuncDefPlanNode *>(plan_ptr);

    ASSERT_TRUE(nullptr != plan->fn_def_);
    ASSERT_TRUE(nullptr != plan->fn_def_->header_);
    ASSERT_TRUE(nullptr != plan->fn_def_->block_);
    ASSERT_EQ(3u, plan->fn_def_->block_->children.size());
    ASSERT_EQ(node::kFnAssignStmt,
              plan->fn_def_->block_->children[0]->GetType());
    ASSERT_EQ(node::kFnAssignStmt,
              plan->fn_def_->block_->children[1]->GetType());
    ASSERT_EQ(node::kFnIfElseBlock,
              plan->fn_def_->block_->children[2]->GetType());

    // validate select plan
    plan_ptr = trees[1];
    ASSERT_TRUE(NULL != plan_ptr);
    std::cout << *plan_ptr << std::endl;
    ASSERT_EQ(node::kPlanTypeQuery, plan_ptr->GetType());
    plan_ptr = plan_ptr->GetChildren()[0];
    ASSERT_EQ(node::kPlanTypeLimit, plan_ptr->GetType());
    node::LimitPlanNode *limit_plan =
        dynamic_cast<node::LimitPlanNode *>(plan_ptr);
    ASSERT_EQ(1, limit_plan->GetLimitCnt());
}

TEST_F(PlannerTest, FunDefIfElseComplexPlanTest) {
    const std::string sql_str =
        "%%fun\n"
        "def test(x:i32,y:i32):i32\n"
        "    if x > 1\n"
        "    \tc=x+y\n"
        "    elif y >1\n"
        "    \tif x-y >0\n"
        "    \t\td=x-y\n"
        "    \t\tc=d+1\n"
        "    \telif x-y <0\n"
        "    \t\tc = y-x\n"
        "    \telse\n"
        "    \t\tc = 9999\n"
        "    else\n"
        "    \tif x < -100\n"
        "    \t\tc = x+100\n"
        "    \telif y < -100\n"
        "    \t\tc = y+100\n"
        "    \telse\n"
        "    \t\tc=x*y\n"
        "    return c\n"
        "end\n"
        "%%sql\n"
        "select col1, test(col1, col2) from t1 limit 1;";
    std::cout << sql_str;

    node::NodePointVector list;
    node::PlanNodeList trees;
    base::Status status;
    int ret = parser_->parse(sql_str, list, manager_, status);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(2u, list.size());
    std::cout << *(list[0]) << std::endl;
    std::cout << *(list[1]) << std::endl;

    Planner *planner_ptr = new SimplePlanner(manager_);
    ASSERT_EQ(0, planner_ptr->CreatePlanTree(list, trees, status));
    std::cout << status.msg << std::endl;
    ASSERT_EQ(2u, trees.size());
    PlanNode *plan_ptr = trees[0];
    ASSERT_TRUE(NULL != plan_ptr);
    std::cout << *plan_ptr << std::endl;

    // validate fundef plan
    ASSERT_EQ(node::kPlanTypeFuncDef, plan_ptr->GetType());
    node::FuncDefPlanNode *plan =
        dynamic_cast<node::FuncDefPlanNode *>(plan_ptr);

    ASSERT_TRUE(nullptr != plan->fn_def_);
    ASSERT_TRUE(nullptr != plan->fn_def_->header_);
    ASSERT_TRUE(nullptr != plan->fn_def_->block_);
    ASSERT_EQ(2u, plan->fn_def_->block_->children.size());
    ASSERT_EQ(node::kFnIfElseBlock,
              plan->fn_def_->block_->children[0]->GetType());
    ASSERT_EQ(node::kFnReturnStmt,
              plan->fn_def_->block_->children[1]->GetType());

    {
        node::FnIfElseBlock *block = dynamic_cast<node::FnIfElseBlock *>(
            plan->fn_def_->block_->children[0]);
        // if block check: if x>1
        {
            ASSERT_EQ(node::kExprBinary,
                      block->if_block_->if_node->expression_->GetExprType());
            // c = x+y
            ASSERT_EQ(1u, block->if_block_->block_->children.size());
        }
        ASSERT_EQ(1u, block->elif_blocks_.size());

        {
            // elif block check: elif y>1
            ASSERT_EQ(node::kFnElifBlock, block->elif_blocks_[0]->GetType());
            node::FnElifBlock *elif_block =
                dynamic_cast<node::FnElifBlock *>(block->elif_blocks_[0]);
            ASSERT_EQ(node::kExprBinary,
                      elif_block->elif_node_->expression_->GetExprType());
            ASSERT_EQ(1u, elif_block->block_->children.size());
            ASSERT_EQ(node::kFnIfElseBlock,
                      elif_block->block_->children[0]->GetType());
            // check if elif else block
            {
                node::FnIfElseBlock *block =
                    dynamic_cast<node::FnIfElseBlock *>(
                        elif_block->block_->children[0]);
                // check if x-y>0
                //          c = x-y
                {
                    ASSERT_EQ(
                        node::kExprBinary,
                        block->if_block_->if_node->expression_->GetExprType());
                    // c = x-y
                    ASSERT_EQ(2u, block->if_block_->block_->children.size());
                    ASSERT_EQ(node::kFnAssignStmt,
                              block->if_block_->block_->children[0]->GetType());
                    ASSERT_TRUE(dynamic_cast<node::FnAssignNode *>(
                                    block->if_block_->block_->children[0])
                                    ->IsSSA());
                    ASSERT_EQ(node::kFnAssignStmt,
                              block->if_block_->block_->children[1]->GetType());
                    ASSERT_FALSE(dynamic_cast<node::FnAssignNode *>(
                                     block->if_block_->block_->children[1])
                                     ->IsSSA());
                }
                ASSERT_EQ(1u, block->elif_blocks_.size());
                // check elif x-y<0
                //          c = y-x
                {
                    ASSERT_EQ(node::kFnElifBlock,
                              block->elif_blocks_[0]->GetType());
                    node::FnElifBlock *elif_block =
                        dynamic_cast<node::FnElifBlock *>(
                            block->elif_blocks_[0]);
                    ASSERT_EQ(
                        node::kExprBinary,
                        elif_block->elif_node_->expression_->GetExprType());
                    ASSERT_EQ(1u, elif_block->block_->children.size());
                    ASSERT_EQ(node::kFnAssignStmt,
                              elif_block->block_->children[0]->GetType());
                }
                // check c = 9999
                ASSERT_EQ(1u, block->else_block_->block_->children.size());
                ASSERT_EQ(node::kFnAssignStmt,
                          block->else_block_->block_->children[0]->GetType());
            }
        }
        // else block check
        {
            ASSERT_EQ(1u, block->else_block_->block_->children.size());
            ASSERT_EQ(node::kFnIfElseBlock,
                      block->else_block_->block_->children[0]->GetType());
        }
    }
    // validate select plan
    plan_ptr = trees[1];
    ASSERT_TRUE(NULL != plan_ptr);
    std::cout << *plan_ptr << std::endl;
    ASSERT_EQ(node::kPlanTypeQuery, plan_ptr->GetType());
    plan_ptr = plan_ptr->GetChildren()[0];
    ASSERT_EQ(node::kPlanTypeLimit, plan_ptr->GetType());
    node::LimitPlanNode *limit_plan =
        dynamic_cast<node::LimitPlanNode *>(plan_ptr);
    ASSERT_EQ(1, limit_plan->GetLimitCnt());
}

TEST_F(PlannerTest, FunDefForInPlanTest) {
    const std::string sql_str =
        "%%fun\n"
        "def test(l:list<i32>, a:i32):i32\n"
        "    sum=0\n"
        "    for x in l\n"
        "        if x > a\n"
        "            sum = sum + x\n"
        "    return sum\n"
        "end\n"
        "%%sql\n"
        "select col1, test(col1, col2) from t1 limit 1;";
    std::cout << sql_str;

    node::NodePointVector list;
    node::PlanNodeList trees;
    base::Status status;
    int ret = parser_->parse(sql_str, list, manager_, status);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(2u, list.size());
    std::cout << *(list[0]) << std::endl;
    std::cout << *(list[1]) << std::endl;

    Planner *planner_ptr = new SimplePlanner(manager_);
    ASSERT_EQ(0, planner_ptr->CreatePlanTree(list, trees, status));
    std::cout << status.msg << std::endl;
    ASSERT_EQ(2u, trees.size());
    PlanNode *plan_ptr = trees[0];
    ASSERT_TRUE(NULL != plan_ptr);
    std::cout << *plan_ptr << std::endl;

    // validate fundef plan
    ASSERT_EQ(node::kPlanTypeFuncDef, plan_ptr->GetType());
    node::FuncDefPlanNode *plan =
        dynamic_cast<node::FuncDefPlanNode *>(plan_ptr);

    ASSERT_TRUE(nullptr != plan->fn_def_);
    ASSERT_TRUE(nullptr != plan->fn_def_->header_);
    ASSERT_TRUE(nullptr != plan->fn_def_->block_);
    ASSERT_EQ(3u, plan->fn_def_->block_->children.size());

    // validate udf plan
    ASSERT_EQ(node::kFnAssignStmt,
              plan->fn_def_->block_->children[0]->GetType());
    ASSERT_EQ(node::kFnForInBlock,
              plan->fn_def_->block_->children[1]->GetType());
    // validate for in block
    {
        node::FnForInBlock *for_block = dynamic_cast<node::FnForInBlock *>(
            plan->fn_def_->block_->children[1]);
        ASSERT_EQ(1u, for_block->block_->children.size());
        ASSERT_EQ(node::kFnIfElseBlock,
                  for_block->block_->children[0]->GetType());
    }
    // validate select plan
    plan_ptr = trees[1];
    ASSERT_TRUE(NULL != plan_ptr);
    std::cout << *plan_ptr << std::endl;
    ASSERT_EQ(node::kPlanTypeQuery, plan_ptr->GetType());
    plan_ptr = plan_ptr->GetChildren()[0];
    ASSERT_EQ(node::kPlanTypeLimit, plan_ptr->GetType());
    node::LimitPlanNode *limit_plan =
        dynamic_cast<node::LimitPlanNode *>(plan_ptr);
    ASSERT_EQ(1, limit_plan->GetLimitCnt());
}
}  // namespace plan
}  // namespace fesql

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
