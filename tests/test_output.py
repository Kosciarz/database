import os
import sys
import subprocess
import unittest
import tempfile


path = f"{sys.argv[1]}"


class TestOutput(unittest.TestCase):
    def test_allows_printing_structure_3_leaf_btree(self):
        def extract_output(output: str):
            start_index = output.find("Tree:")
            end_index = output[start_index:].find(".")
            return output[start_index:start_index + end_index + 1].strip()
    
        
        input = "".join([f"insert {i} user{i} person{i}@example.com\n" for i in range(1, 15)])
        input += """
.btree\n
insert 15 user15 user15@example.com\n
.exit\n"""

        expected = """
Tree:
- internal (size 1)
  - leaf (size 7)
    - 1
    - 2
    - 3
    - 4
    - 5
    - 6
    - 7
  - key 7
  - leaf (size 7)
    - 8
    - 9
    - 10
    - 11
    - 12
    - 13
    - 14
database> database> Executed."""

        with tempfile.NamedTemporaryFile(delete=False) as tmp:
            temp_file_path = tmp.name    

        process = subprocess.run(
            [path, temp_file_path],
	        input=input,
	        text=True,
	        capture_output=True
        )
        
        os.remove(temp_file_path)
        self.assertEqual(expected.strip(), extract_output(process.stdout))

    def test_allows_inserting_max_rows(self):
        def extract_output(output: str):
            start_index = output.rfind("database> Executed.")
            return output[start_index:].strip()
            
        input = "".join([f"insert {i} user{i} person{i}@example.com\n" for i in range(1, 1401)])

        expected = """
database> Executed.
database> Need to implement updating parent after spliting."""
        
        with tempfile.NamedTemporaryFile(delete=False) as tmp:
            temp_file_path = tmp.name
        
        process = subprocess.run(
            [path, temp_file_path],
            input=input,
            text=True,
            capture_output=True
        )

        os.remove(temp_file_path)
        self.assertEqual(expected.strip(), extract_output(process.stdout))

    def test_prints_all_rows_in_multi_level_tree(self):
        def extract_output(output: str):
            start_index = output.find("(")
            return output[start_index:].strip()

        input = "".join(f"insert {i} user{i} person{i}@example.com\n" for i in range(1, 16))
        input += "select\n"
        input += ".exit\n"

        expected = """
(1, user1, person1@example.com)
(2, user2, person2@example.com)
(3, user3, person3@example.com)
(4, user4, person4@example.com)
(5, user5, person5@example.com)
(6, user6, person6@example.com)
(7, user7, person7@example.com)
(8, user8, person8@example.com)
(9, user9, person9@example.com)
(10, user10, person10@example.com)
(11, user11, person11@example.com)
(12, user12, person12@example.com)
(13, user13, person13@example.com)
(14, user14, person14@example.com)
(15, user15, person15@example.com)
Executed.
database>"""

        with tempfile.NamedTemporaryFile(delete=False) as tmp:
            temp_file_path = tmp.name

        process = subprocess.run(
            [path, temp_file_path],
            input=input,
            text=True,
            capture_output=True
        )

        os.remove(temp_file_path)
        self.assertEqual(expected.strip(), extract_output(process.stdout.strip()))


if __name__ == "__main__":
    unittest.main(argv=[""], exit=False)
