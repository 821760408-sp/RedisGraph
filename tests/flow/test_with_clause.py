from RLTest import Env
from redisgraph import Graph, Node, Edge
from base import FlowTestsBase

redis_graph = None
values = ["str1", "str2", False, True, 5, 10.5]

class testWithClause(FlowTestsBase):
    
    def __init__(self):
        super(testWithClause, self).__init__()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("G", redis_con)
        self.populate_graph()

    
    def populate_graph(self):
        # Populate a graph with two labels, each containing the same property values but different keys.
        # Each node pair is connected by an edge from label_a to label_b
        a_nodes = []
        for idx, v in enumerate(values):
            node = Node(label="label_a", properties={"a_val": v, "a_idx": idx})
            a_nodes.append(node)
            redis_graph.add_node(node)
        for idx, v in enumerate(values):
            node = Node(label="label_b", properties={"b_val": v, "b_idx": idx})
            redis_graph.add_node(node)
            edge = Edge(a_nodes[idx], 'connects', node, properties={"edgeval": idx})
            redis_graph.add_edge(edge)

        redis_graph.commit()
    
    # Verify that graph entities specified in a WITH clause are returned properly
    def test01_with_scalar_read_queries(self):
        query = """MATCH (a:label_a) WITH a.a_val AS val RETURN val ORDER BY val"""
        actual_result = redis_graph.query(query)
        expected = [['str1'],
                    ['str2'],
                    [False],
                    [True],
                    [5],
                    [10.5]]
        self.env.assertEqual(actual_result.result_set, expected)


        query = """MATCH (a:label_a) WITH a.a_val AS val SKIP 1 LIMIT 1 RETURN val ORDER BY val"""
        actual_result = redis_graph.query(query)
        expected = [['str2']]
        self.env.assertEqual(actual_result.result_set, expected)


        query = """MERGE (a:label_a {a_val: 5}) WITH a.a_val AS val RETURN val ORDER BY val"""
        actual_result = redis_graph.query(query)
        expected = [[5]]
        self.env.assertEqual(actual_result.properties_set, 0)

        self.env.assertEqual(actual_result.result_set, expected)


        # Merge on existing edge
        query = """MERGE ()-[e {edgeval: 5}]->() WITH e.edgeval AS val RETURN val"""
        actual_result = redis_graph.query(query)
        expected = [[5]]
        self.env.assertEqual(actual_result.properties_set, 0)

        self.env.assertEqual(actual_result.result_set, expected)
    def test02_with_arithmetic_op_read_queries(self):
        # Iterate over nodes
        query = """MATCH (a) WITH ID(a) AS id RETURN id ORDER BY id"""
        actual_result = redis_graph.query(query)
        expected = [[0],
                    [1],
                    [2],
                    [3],
                    [4],
                    [5],
                    [6],
                    [7],
                    [8],
                    [9],
                    [10],
                    [11]]
        self.env.assertEqual(actual_result.result_set, expected)

        # Iterate over edges
        query = """MATCH ()-[e]->() WITH ID(e) AS id RETURN id ORDER BY id"""
        actual_result = redis_graph.query(query)

        expected = [[0],
                    [1],
                    [2],
                    [3],
                    [4],
                    [5]]
        self.env.assertEqual(actual_result.result_set, expected)

        # Perform chained arithmetic ops
        query = """MATCH (a)-[]->(b) WHERE a.a_val > 0 AND b.b_val > 0 WITH a.a_val * 2 + b.b_val AS val RETURN val"""
        actual_result = redis_graph.query(query)

        expected = [[31.5],
                    [15]]
        self.env.assertEqual(actual_result.result_set, expected)

    def test03_with_aggregate_op_read_queries(self):
        query = """MATCH (a)-[e]->() WITH COUNT(a.a_val) AS count_res, SUM(ID(e)) AS sum_res RETURN count_res, sum_res"""
        actual_result = redis_graph.query(query)
        expected = [[6, 15]]
        self.env.assertEqual(actual_result.result_set, expected)

    # TODO UNWIND support needs to be extended for combinations like UNWIND...MATCH
    def test04_with_unwind_expressions(self):
        query = """UNWIND [1, 2, 3] AS x WITH x AS y RETURN y"""
        actual_result = redis_graph.query(query)

        expected = [[1],
                    [2],
                    [3]]
        self.env.assertEqual(actual_result.result_set, expected)

        query = """UNWIND [1, 2, 3] AS x WITH x * 2 AS y RETURN y"""
        actual_result = redis_graph.query(query)

        expected = [[2],
                    [4],
                    [6]]
        self.env.assertEqual(actual_result.result_set, expected)

        query = """UNWIND [1, 2, 3] AS x WITH x * 2 AS y WITH y * 2 AS z RETURN z"""
        actual_result = redis_graph.query(query)

        expected = [[4],
                    [8],
                    [12]]
        self.env.assertEqual(actual_result.result_set, expected)

        query = """UNWIND [1, 2, 3] AS x WITH x * 2 AS y WITH y * 2 AS z WITH MIN(z) as min RETURN min"""
        actual_result = redis_graph.query(query)

        expected = [[4]]
        self.env.assertEqual(actual_result.result_set, expected)


    def test05_with_create_expressions(self):
        query = """CREATE (c:c_label {c_val: 25}) WITH c AS c RETURN c.c_val AS val"""
        actual_result = redis_graph.query(query)
        expected = [[25]]
        self.env.assertEqual(actual_result.result_set, expected)
        self.env.assertEqual(actual_result.labels_added, 1)
        self.env.assertEqual(actual_result.nodes_created, 1)
        self.env.assertEqual(actual_result.properties_set, 1)

        # TODO Update CREATE to accept variable arguments from UNWIND, WITH, etc
        query = """UNWIND [5] AS a WITH a AS b CREATE (:unwind_label {prop: 'some_constant'})"""
        actual_result = redis_graph.query(query)

        self.env.assertEqual(actual_result.labels_added, 1)
        self.env.assertEqual(actual_result.nodes_created, 1)
        self.env.assertEqual(actual_result.properties_set, 1)
        query = """MATCH (a:unwind_label) RETURN a.prop"""
        actual_result = redis_graph.query(query)

        # Verify that the entity was created properly
        expected = [['some_constant']]
        self.env.assertEqual(actual_result.result_set, expected)

    def test06_update_expressions(self):
        query = """MATCH (c:c_label) SET c.c_val = 50 WITH c.c_val AS val RETURN val"""
        actual_result = redis_graph.query(query)
        expected = [[50]]
        self.env.assertEqual(actual_result.result_set, expected)
        self.env.assertEqual(actual_result.properties_set, 1)
