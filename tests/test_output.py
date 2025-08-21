import subprocess, sys


path = f"{sys.argv[1]}"
print(path)

input = '''
insert 1 1 1\n
insert 2 1 1\n
insert 3 1 1\n
insert 4 1 1\n
insert 5 1 1\n
insert 6 1 1\n
insert 7 1 1\n
insert 8 1 1\n
insert 9 1 1\n
insert 10 1 1\n
insert 11 1 1\n
insert 12 1 1\n
insert 13 1 1\n
insert 14 1 1\n
.btree\n
'''

process = subprocess.run(
	[path],
	input=input,
	text=True,
	capture_output=True
)

expected = '''Tree:
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
    - 14'''


def extract_output(output: str):
    start_index = output.find("Tree:")
    end_index = output[start_index:].find("database")
    return output[start_index:start_index + end_index].strip()

extracted_ouput = extract_output(process.stdout)

if extracted_ouput == expected.strip():
    print("Passed!")
    sys.exit(0)
else:
	print("Expected:\n", expected)
	print("Got:\n", extracted_ouput)
	sys.exit(1)
