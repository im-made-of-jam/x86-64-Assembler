# builds all of the cpp files from this directory (add some more in future)
# keeps track of hashes of files so that only modified files are recompiled

import os

def gccCompile(thisFile):
	os.system(f"""powershell "Invoke-Expression 'g++ -std=gnu++26 -c {thisFile} -o ./.objectFiles/{thisFile.replace("cpp", "o")}'" """)

if (__name__ == "__main__"):
	import hashlib
	import multiprocessing
	import sys

	nocache = bool((len(sys.argv) >= 2) and (sys.argv[1] == "nocache"))

	# get the hash of one file
	def getHash(filename):
		md5 = hashlib.md5()

		with open(filename, "rb") as openFile:
			while True:
				data = openFile.read(1024)
				# no data means we have read all of the file
				if not data:
					break

				md5.update(data)

		return md5.hexdigest()

	potentialCompiles = [] # every file we could be compiling
	filesToCompile = []    # every file we are actually compiling
	hashesOfFilesOnDisk = {} # the hash of every file we could be compiling

	# get every .cpp file in this directory
	for file in os.listdir(os.getcwd()):
		if file.split(".")[-1] == "cpp":
			potentialCompiles.append(file)
			filesToCompile.append(file)

	# get the hashes of the files on disk
	for filename in potentialCompiles:
		hashesOfFilesOnDisk[filename] = getHash(filename)

	if not (os.path.exists("./.build_py_fileHashes.dat") and os.path.isfile("./.build_py_fileHashes.dat")):
		with open("./.build_py_fileHashes.dat", "x"):
			# just need to create the file dont need to do anything with it here
			pass

	# now compare against all hashed files so far
	with open("./.build_py_fileHashes.dat", "rt+") as hashFile:
		name = "placeholder"

		while name:
			# get the name hash pair
			name = hashFile.readline().replace('\n', "")
			hash = hashFile.readline().replace('\n', "")

			try:
				# if the file hasnt changed
				if (hashesOfFilesOnDisk[name] == hash) and (not nocache):
					# dont need to compile it
					filesToCompile.remove(name)

			except KeyError:
				# dont have a hash of this file yet
				pass

		# now write every hash we have calculated for potential compile files
		hashFile.truncate(0)
		hashFile.seek(0)
		for name in potentialCompiles:
			hashFile.write(name)
			hashFile.write('\n')
			hashFile.write(hashesOfFilesOnDisk[name])
			hashFile.write('\n')

	for filename in filesToCompile:
		print(f"compiling {filename}")

	if len(filesToCompile) != 0:
		with multiprocessing.Pool(processes=len(filesToCompile)) as pool:
			pool.map(gccCompile, filesToCompile)

	print("compiling object files")
	os.system("""powershell "Invoke-Expression 'g++ ./.objectFiles/*.o -o ./main' """)
