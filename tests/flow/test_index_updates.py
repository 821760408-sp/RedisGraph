import os
import sys
import random
import string
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

GRAPH_ID = "index_test"
redis_graph = None
labels = ["label_a", "label_b"]
fields = ['unique', 'group', 'doubleval', 'intval', 'stringval']
groups = ["Group A", "Group B", "Group C","Group D", "Group E"]
node_ctr = 0

class testIndexUpdatesFlow(FlowTestsBase):
    def __init__(self):
        super(testIndexUpdatesFlow, self).__init__()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()
        self.build_indices() 

    def new_node(self):
        return Node(label = labels[node_ctr % 2],
                    properties = {'unique': node_ctr,
                                  'group': random.choice(groups),
                                  'doubleval': round(random.uniform(-1, 1), 2),
                                  'intval': random.randint(1, 10000),
                                  'stringval': ''.join(random.choice(string.lowercase) for x in range(6))})

    def populate_graph(self):
        global node_ctr
        for i in range(1000):
            node = self.new_node()
            redis_graph.add_node(node)
            node_ctr += 1
        redis_graph.commit()
    
    def build_indices(self):
        for field in fields:
            redis_graph.redis_con.execute_command("GRAPH.QUERY", GRAPH_ID, "CREATE INDEX ON :label_a(%s)" % (field))
            redis_graph.redis_con.execute_command("GRAPH.QUERY", GRAPH_ID, "CREATE INDEX ON :label_b(%s)" % (field))

    # Validate that all properties are indexed
    def validate_indexed(self):
        for field in fields:
            resp = redis_graph.execution_plan("""MATCH (a:label_a) WHERE a.%s > 0 RETURN a""" % (field))
            self.env.assertIn('Index Scan', resp)

    # So long as 'unique' is not modified, label_a.unique will always be even and label_b.unique will always be odd
    def validate_unique(self):
        result = redis_graph.query("MATCH (a:label_a) RETURN a.unique")
        # Remove the header
        result.result_set.pop(0)
        for val in result.result_set:
            self.env.assertEquals(int(float(val[0])) % 2, 0)

        result = redis_graph.query("MATCH (b:label_b) RETURN b.unique")
        # Remove the header
        result.result_set.pop(0)
        for val in result.result_set:
            self.env.assertEquals(int(float(val[0])) % 2, 1)

    # The index scan ought to return identical results to a label scan over the same range of values.
    def validate_doubleval(self):
        for label in labels:
            resp = redis_graph.execution_plan("""MATCH (a:%s) WHERE a.doubleval < 100 RETURN a.doubleval ORDER BY a.doubleval""" % (label))
            self.env.assertIn('Index Scan', resp)
            indexed_result = redis_graph.query("""MATCH (a:%s) WHERE a.doubleval < 100 RETURN a.doubleval ORDER BY a.doubleval""" % (label))
            scan_result = redis_graph.query("""MATCH (a:%s) RETURN a.doubleval ORDER BY a.doubleval""" % (label))

            self.env.assertEqual(len(indexed_result.result_set), len(scan_result.result_set))
            # Collect any elements between the two result sets that fail a string comparison
            # so that we may compare them as doubles (specifically, -0 and 0 should be considered equal)
            differences = [[i[0], j[0]] for i, j in zip(indexed_result.result_set, scan_result.result_set) if i != j]
            for pair in differences:
                self.env.assertEqual(float(pair[0]), float(pair[1]))

    # The intval property can be assessed similar to doubleval, but the result sets should be identical
    def validate_intval(self):
        for label in labels:
            resp = redis_graph.execution_plan("""MATCH (a:%s) WHERE a.intval > 0 RETURN a.intval ORDER BY a.intval""" % (label))
            self.env.assertIn('Index Scan', resp)
            indexed_result = redis_graph.query("""MATCH (a:%s) WHERE a.intval > 0 RETURN a.intval ORDER BY a.intval""" % (label))
            scan_result = redis_graph.query("""MATCH (a:%s) RETURN a.intval ORDER BY a.intval""" % (label))

            self.env.assertEqual(indexed_result.result_set, scan_result.result_set)

    # Validate a series of premises to ensure that the graph has not been modified unexpectedly
    def validate_state(self):
        self.validate_unique()
        self.validate_indexed()
        self.validate_doubleval()
        self.validate_intval()

    # Modify a property, triggering updates to all nodes in two indices
    def test01_full_property_update(self):
        result = redis_graph.query("MATCH (a) SET a.doubleval = a.doubleval + %f" % (round(random.uniform(-1, 1), 2)))
        self.env.assertEquals(result.properties_set, 1000)
        # Verify that index scans still function and return correctly
        self.validate_state()

    # Modify a property, triggering updates to a subset of nodes in two indices
    def test02_partial_property_update(self):
        redis_graph.query("MATCH (a) WHERE a.doubleval > 0 SET a.doubleval = a.doubleval + %f" % (round(random.uniform(-1, 1), 2)))
        # Verify that index scans still function and return correctly
        self.validate_state()

    #  Add 100 randomized nodes and validate indices
    def test03_node_creation(self):
        # Reset nodes in the Graph object so that we won't double-commit the originals
        redis_graph.nodes = {}
        global node_ctr
        for i in range(100):
            node = self.new_node()
            redis_graph.add_node(node)
            node_ctr += 1
        redis_graph.commit()
        self.validate_state()

    # Delete every other node in first 100 and validate indices
    def test04_node_deletion(self):
        # Reset nodes in the Graph object so that we won't double-commit the originals
        redis_graph.nodes = {}
        global node_ctr
        # Delete nodes one at a time
        for i in range(0, 100, 2):
            result = redis_graph.query("MATCH (a) WHERE ID(a) = %d DELETE a" % (i))
            self.env.assertEquals(result.nodes_deleted, 1)
            node_ctr -= 1
        self.validate_state()

        # Delete all nodes matching a filter
        result = redis_graph.query("MATCH (a:label_a) WHERE a.group = 'Group A' DELETE a")
        self.env.assertGreater(result.nodes_deleted, 0)
        self.validate_state()
