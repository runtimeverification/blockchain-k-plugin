pipeline {
  agent {
    dockerfile {
      additionalBuildArgs '--build-arg USER_ID=$(id -u) --build-arg GROUP_ID=$(id -g)'
    }
  }
  options {
    ansiColor('xterm')
  }
  stages {
    stage("Init title") {
      when { changeRequest() }
      steps {
        script {
          currentBuild.displayName = "PR ${env.CHANGE_ID}: ${env.CHANGE_TITLE}"
        }
      }
    }
    stage("Test compilation") {
      steps {
        dir ('llvm-backend') {
          git url: 'git@github.com:kframework/llvm-backend.git'
          sh '''
            mkdir build
            cd build
            cmake .. -DCMAKE_BUILD_TYPE=Release
            make include
          '''
        }
        sh 'make -j16'
      }
    }
  }
}
