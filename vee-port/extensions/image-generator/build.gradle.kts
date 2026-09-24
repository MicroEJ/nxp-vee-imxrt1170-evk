/*
 * Kotlin
 *
 * Copyright 2026 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
 
plugins {
    id("com.microej.gradle.jse-library")
}

microej {
    skippedCheckers = "changelog,readme,license,nullanalysis"
    additionalFilesDir.set(rootProject.layout.projectDirectory)
}

dependencies {
    implementation(libs.pack.ui) {
        artifact {
            name = "imageGenerator"
            extension = "jar"
        }
    }
}

// Image Generator jar name must start with "imageGenerator" to be correctly loaded by the VEE Port
tasks.jar {
    archiveBaseName = "imageGenerator-mimxrt1170"
}