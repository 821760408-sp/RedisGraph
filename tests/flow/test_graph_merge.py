import os
import sys
from redisgraph import Graph, Node, Edge
from base import FlowTestsBase

redis_graph = None

class testGraphMergeFlow(FlowTestsBase):
    def __init__(self):
        super(testGraphMergeFlow, self).__init__()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("G", redis_con)

    # Create a single node without any labels or properties.
    def test01_single_node_with_label(self):
        global redis_graph
        query = """MERGE (robert:Critic)"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.labels_added, 1)
        self.env.assertEquals(result.nodes_created, 1)
        self.env.assertEquals(result.properties_set, 0)

    # Retry to create an existing entity.
    def test02_existing_single_node_with_label(self):
        global redis_graph
        query = """MERGE (robert:Critic)"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.properties_set, 0)
    
    # Create a single node with two properties and no labels.
    def test03_single_node_with_properties(self):
        global redis_graph
        query = """MERGE (charlie { name: 'Charlie Sheen', age: 10 })"""
        result = redis_graph.query(query)        
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 1)
        self.env.assertEquals(result.properties_set, 2)
    
    # Retry to create an existing entity.
    def test04_existing_single_node_with_properties(self):
        global redis_graph
        query = """MERGE (charlie { name: 'Charlie Sheen', age: 10 })"""
        result = redis_graph.query(query)        
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.properties_set, 0)
    
    # Create a single node with both label and property.
    def test05_single_node_both_label_and_property(self):
        global redis_graph
        query = """MERGE (michael:Person { name: 'Michael Douglas' })"""
        result = redis_graph.query(query)        
        self.env.assertEquals(result.labels_added, 1)
        self.env.assertEquals(result.nodes_created, 1)
        self.env.assertEquals(result.properties_set, 1)
    
    # Retry to create an existing entity.
    def test06_existing_single_node_both_label_and_property(self):
        global redis_graph
        query = """MERGE (michael:Person { name: 'Michael Douglas' })"""
        result = redis_graph.query(query)        
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.properties_set, 0)

    # Create a single edge and additional two nodes.
    def test07_merge_on_relationship(self):
        global redis_graph
        query = """MERGE (charlie:ACTOR)-[r:ACTED_IN]->(wallStreet:MOVIE)"""
        result = redis_graph.query(query)        
        self.env.assertEquals(result.labels_added, 2)
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.properties_set, 0)
        self.env.assertEquals(result.relationships_created, 1)
    
    # Retry to create a single edge and additional two nodes.
    def test08_existing_merge_on_relationship(self):
        global redis_graph
        query = """MERGE (charlie:ACTOR)-[r:ACTED_IN]->(wallStreet:MOVIE)"""
        result = redis_graph.query(query)        
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.properties_set, 0)
        self.env.assertEquals(result.relationships_created, 0)

    # Update existing entity
    def test09_update_existing_node(self):
        global redis_graph
        query = """MERGE (charlie { name: 'Charlie Sheen', age: 10 }) SET charlie.age = 11, charlie.lastname='Sheen' """
        result = redis_graph.query(query)
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.properties_set, 2)
        self.env.assertEquals(result.relationships_created, 0)

        query = """MATCH (charlie { name: 'Charlie Sheen' }) RETURN charlie.age, charlie.name, charlie.lastname"""
        actual_result = redis_graph.query(query)
        expected_result = [[11, 'Charlie Sheen', 'Sheen']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Update new entity
    def test10_update_new_node(self):
        global redis_graph
        query = """MERGE (tamara:ACTOR { name: 'tamara tunie' }) SET tamara.age = 59, tamara.name = 'Tamara Tunie' """
        result = redis_graph.query(query)
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 1)
        self.env.assertEquals(result.properties_set, 3)
        self.env.assertEquals(result.relationships_created, 0)

        query = """MATCH (tamara:ACTOR { name: 'Tamara Tunie' }) RETURN tamara.name, tamara.age"""
        actual_result = redis_graph.query(query)
        expected_result = [['Tamara Tunie', 59]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Create a single edge and additional two nodes.
    def test11_update_new_relationship(self):
        global redis_graph
        query = """MERGE (franklin:ACTOR { name: 'Franklin Cover' })-[r:ACTED_IN {rate:5.7}]->(almostHeroes:MOVIE) SET r.date=1998, r.rate=5.8"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.properties_set, 4)
        self.env.assertEquals(result.relationships_created, 1)
    
    # Update existing relation
    def test12_update_existing_edge(self):
        global redis_graph
        query = """MERGE (franklin:ACTOR { name: 'Franklin Cover' })-[r:ACTED_IN {rate:5.8, date:1998}]->(almostHeroes:MOVIE) SET r.date=1998, r.rate=5.9"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.properties_set, 2)
        self.env.assertEquals(result.relationships_created, 0)

        query = """MATCH (franklin:ACTOR { name: 'Franklin Cover' })-[r:ACTED_IN {rate:5.9, date:1998}]->(almostHeroes:MOVIE) RETURN franklin.name, franklin.age, r.rate, r.date"""
        actual_result = redis_graph.query(query)
        expected_result = [['Franklin Cover', None, 5.9, 1998]]
        self.env.assertEquals(actual_result.result_set, expected_result)
    
    # Update multiple nodes
    def test13_update_multiple_nodes(self):
        global redis_graph
        query = """CREATE (:person {age:31}),(:person {age:31}),(:person {age:31}),(:person {age:31})"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.labels_added, 1)
        self.env.assertEquals(result.nodes_created, 4)
        self.env.assertEquals(result.properties_set, 4)

        query = """MERGE (p:person {age:31}) SET p.newprop=100"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.properties_set, 4)

        query = """MATCH (p:person) RETURN p.age, p.newprop"""
        actual_result = redis_graph.query(query)
        expected_result = [[31, 100],
                           [31, 100],
                           [31, 100],
                           [31, 100]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Update multiple nodes
    def test14_merge_unbounded_pattern(self):
        global redis_graph
        query = """MERGE (p:person {age:31})-[:owns]->(d:dog {name:'max'})"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.labels_added, 1)
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.properties_set, 2)
        self.env.assertEquals(result.relationships_created, 1)

        # Although person with age 31 and dog with the name max exists,
        # specified pattern doesn't exists, as a result the entire pattern
        # will be created, if we were to support MATCH MERGE 'p' and 'd'
        # would probably be defined in the MATCH clause, as a result they're
        # bounded and won't be duplicated.
        query = """MERGE (p:person {age:31})-[:owns]->(d:dog {name:'max'})-[:eats]->(f:food {name:'Royal Canin'})"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.labels_added, 1)
        self.env.assertEquals(result.nodes_created, 3)
        self.env.assertEquals(result.properties_set, 3)
        self.env.assertEquals(result.relationships_created, 2)

    # Add node that matches pre-existing index
    def test15_merge_indexed_entity(self):
        global redis_graph
        # Create index
        query = """CREATE INDEX ON :person(age)"""
        redis_graph.query(query)

        count_query = """MATCH (p:person) WHERE p.age > 0 RETURN COUNT(p)"""
        result = redis_graph.query(count_query)
        original_count  = result.result_set[0][0]

        # Add one new person
        merge_query = """MERGE (p:person {age:40})"""
        result = redis_graph.query(merge_query)
        self.env.assertEquals(result.nodes_created, 1)
        self.env.assertEquals(result.properties_set, 1)
        # Verify that one indexed node has been added
        result = redis_graph.query(count_query)
        updated_count = result.result_set[0][0]
        self.env.assertEquals(updated_count, original_count+1)

        # Perform another merge that does not create an entity
        result = redis_graph.query(merge_query)
        self.env.assertEquals(result.nodes_created, 0)

        # Verify that indexed node count is unchanged
        result = redis_graph.query(count_query)
        updated_count = result.result_set[0][0]
        self.env.assertEquals(updated_count, original_count+1)
